#include <iostream>
#include <sys/types.h>
#include <sys/stat.h>
#include <map>
#include <vector>
#include <set>

using namespace std;

#include "surfacecorrespondence.h"
#include "gridcreate.h"
#include "boundarycheck.h"
#include "laplacegrid.h"

#include <vtkCellData.h>
#include <vtkCellDerivatives.h>
#include <vtkCellLocator.h>
#include <vtkCleanPolyData.h>
#include <vtkDataArray.h>
#include <vtkDoubleArray.h>
#include <vtkFloatArray.h>
#include <vtkGenericCell.h>
#include <vtkGradientFilter.h>
#include <vtkType.h>
#include <vtkImageData.h>
#include <vtkImageStencil.h>
#include <vtkImageToStructuredGrid.h>
#include <vtkIntArray.h>
#include <vtkMath.h>
#include <vtkModifiedBSPTree.h>
#include <vtkNew.h>
#include <vtkPolyDataNormals.h>
#include <vtkPolyDataToImageStencil.h>
#include <vtkPointData.h>
#include <vtkPointLocator.h>
#include <vtkSphericalTransform.h>
#include <vtkStreamTracer.h>
#include <vtkStructuredGrid.h>
#include <vtkThresholdPoints.h>

#include <vtkImageWriter.h>
#include <vtkMetaImageWriter.h>

SurfaceCorrespondence::SurfaceCorrespondence(string WMsurf, string GMHsurf, int dims /* = 300*/, string output_dir /* = ""*/):
    m_dims(dims)
{
    // Creating output directory if necessary
    int dirstatus = setOutputLocation(output_dir);
    if (dirstatus == EXIT_FAILURE){
        exit(1);
    }

    m_whiteMatterSurface = m_vio.readFile(WMsurf);
    m_greyMatterHull = m_vio.readFile(GMHsurf);
}

SurfaceCorrespondence::SurfaceCorrespondence(vtkPolyData *whiteMatterSurf, vtkPolyData *greyMatterHullSurf, int dims /* = 300*/, string output_dir /* = ""*/):
    m_dims(dims)
{
    // Creating output directory if necessary
    int dirstatus = setOutputLocation(output_dir);
    if (dirstatus == EXIT_FAILURE){
        exit(1);
    }

    m_whiteMatterSurface = whiteMatterSurf;
    m_greyMatterHull = greyMatterHullSurf;
}

void SurfaceCorrespondence::setPrefix(string prefix){
    m_prefix = prefix;
}

int SurfaceCorrespondence::setOutputLocation(string dirname){
    if (dirname[dirname.size()-1] == '/'){
        m_output_dir = dirname;
    }
    else{
        m_output_dir = dirname + '/';
    }

    struct stat info;
    if( stat( m_output_dir.c_str(), &info ) != 0 ){
        if (mkdir(m_output_dir.c_str(), 0777) == -1){
            cerr << "Error :  " << strerror(errno) << endl;
            return EXIT_FAILURE;
        }
        else{
            cout << m_output_dir << " directory created";
            return EXIT_SUCCESS;
        }
    }
    else if( info.st_mode & S_IFDIR ){
        return EXIT_SUCCESS;
    }
    else{
        cerr<<m_output_dir<<" is no directory"<<endl;
        return EXIT_FAILURE;
    }
}

void SurfaceCorrespondence::setWriteOptions(bool writeGridFile, bool writeLaplaceFieldFile, bool writeStreamFile, bool writeWarpedMeshFile, bool writeObjFile){
    m_writeGridFile = writeGridFile;
    m_writeLaplaceFieldFile = writeLaplaceFieldFile;
    m_writeStreamFile = writeStreamFile;
    m_writeWarpedMeshFile = writeWarpedMeshFile;
    m_writeObjFile = writeObjFile;
}

void SurfaceCorrespondence::setWriteOptions(bool writeAll){
    m_writeGridFile = writeAll;
    m_writeLaplaceFieldFile = writeAll;
    m_writeStreamFile = writeAll;
    m_writeWarpedMeshFile = writeAll;
    m_writeObjFile = writeAll;
}

void SurfaceCorrespondence::setPDEparams(int PDElow, int PDEhigh, int PDEiter){
    m_PDElow = PDElow;
    m_PDEhigh = PDEhigh;
    m_PDEiter = PDEiter;
}

vtkPolyData* SurfaceCorrespondence::streams(){
    return m_streams;
}

vtkPolyData* SurfaceCorrespondence::whiteMatterSurface(){
    return m_whiteMatterSurface;
}

void SurfaceCorrespondence::createGrid() {
    GridCreate gc(m_whiteMatterSurface->GetBounds(), m_dims);

    // Creating the binary grids of the WM and the WM+GM structures
    vtkStructuredGrid* goim = gc.createStencil(m_whiteMatterSurface);
    vtkStructuredGrid* woim = gc.createStencil(m_whiteMatterSurface);
	
	BoundaryCheck bc;
    size_t insideCountOut = 0;
    insideCountOut = bc.subtract(goim, woim); // subtraction of both structures to botain a three region map
    cout << "Inside Voxels: " << insideCountOut << endl;

    bc.checkSurface(goim, m_whiteMatterSurface, m_whiteMatterSurface); // boundary errors correction
	woim->Delete();
	
    m_laplaceField = goim;
}

void SurfaceCorrespondence::computeLaplacePDE() {
// Compute Laplace PDE based on the adjacency list and border
    if (m_laplaceField == NULL) {
		cout << "Data input is NULL" << endl;
		return;
	}

    LaplaceGrid grid(m_PDElow, m_PDEhigh, m_laplaceField);
	
	clock_t t1 = clock();
	
	// main iteration loop
    for (int i = 1; i <= m_PDEiter; i++) {
		if (i%500 == 0) {
			cout << "iteration: " << i << "\t";
			clock_t t2 = clock();
			cout << (double) (t2-t1) / CLOCKS_PER_SEC * 1000 << " ms;" << endl;
			t1 = t2;
		}
		grid.computeStep();
	}
	clock_t t2 = clock();
	cout << (double) (t2-t1) / CLOCKS_PER_SEC * 1000 << " ms;" << endl;
	
	// return the solution
    m_laplaceField->GetPointData()->AddArray(grid.solution());
    grid.computeNormals();
}

bool SurfaceCorrespondence::performLineClipping(vtkPolyData* streamLines, vtkModifiedBSPTree* tree, /*int lineId,*/ vtkCell* lineToClip, vtkPoints* outputPoints, vtkCellArray* outputLines, double &length) {
	
	/// - Iterate over all points in a line
	vtkIdList* ids = lineToClip->GetPointIds();
	
	
	/// - Identify a line segment included in the line
	int nIntersections = 0;
	bool foundEndpoint = false;
	std::vector<vtkIdType> idList;
	for (int j = 2; j < ids->GetNumberOfIds(); j++) {
		double p1[3], p2[3];
		streamLines->GetPoint(ids->GetId(j-1), p1);
		streamLines->GetPoint(ids->GetId(j), p2);
		
		// handle initial condition
		if (j == 2) {
			double p0[3];
			streamLines->GetPoint(ids->GetId(0), p0);
			idList.push_back(outputPoints->GetNumberOfPoints());
			outputPoints->InsertNextPoint(p0);
			
			idList.push_back(outputPoints->GetNumberOfPoints());
			outputPoints->InsertNextPoint(p1);
			
			length = sqrt(vtkMath::Distance2BetweenPoints(p0, p1));
		}
		
		int subId;
		double x[3] = {-1,-1,-1};
		double t = 0;
		
		double pcoords[3] = { -1, };
		int testLine = tree->IntersectWithLine(p1, p2, 0.01, t, x, pcoords, subId);
		if (testLine) {
			nIntersections ++;
			if (nIntersections > 0) {
				idList.push_back(outputPoints->GetNumberOfPoints());
				outputPoints->InsertNextPoint(x);
				length += sqrt(vtkMath::Distance2BetweenPoints(p1, x));
				foundEndpoint = true;
				break;
			}
		}
		//        cout << testLine << "; " << x[0] << "," << x[1] << "," << x[2] << endl;
		
		
		idList.push_back(outputPoints->GetNumberOfPoints());
		outputPoints->InsertNextPoint(p2);
		length += sqrt(vtkMath::Distance2BetweenPoints(p1, p2));
	}
	
	if (foundEndpoint) {
		outputLines->InsertNextCell(idList.size(), &idList[0]);
		return true;
	} else {
		outputLines->InsertNextCell(idList.size(), &idList[0]);
	}
	return false;
}

vtkPolyData* SurfaceCorrespondence::performStreamTracerPostProcessing(vtkPolyData* streamLines, vtkPolyData* seedPoints, vtkPolyData* destinationSurface) {
	
	const size_t nInputPoints = seedPoints->GetNumberOfPoints();
	
	// remove useless pointdata information
	streamLines->GetPointData()->Reset();
	streamLines->BuildCells();
	streamLines->BuildLinks();
	
	
	// loop over the cell and compute the length
	int nCells = streamLines->GetNumberOfCells();
	
	/// - Prepare the output as a scalar array
	//    vtkDataArray* streamLineLength = streamLines->GetCellData()->GetScalars("Length");
	
	/// - Prepare the output for the input points
	vtkDoubleArray* streamLineLengthPerPoint = vtkDoubleArray::New();
	streamLineLengthPerPoint->SetNumberOfTuples(nInputPoints);
	streamLineLengthPerPoint->SetName("Length");
	streamLineLengthPerPoint->SetNumberOfComponents(1);
	streamLineLengthPerPoint->FillComponent(0, 0);
	
	vtkIntArray* lineCorrect = vtkIntArray::New();
	lineCorrect->SetName("LineOK");
	lineCorrect->SetNumberOfValues(nInputPoints);
	lineCorrect->FillComponent(0, 0);
	
	seedPoints->GetPointData()->SetScalars(streamLineLengthPerPoint);
	seedPoints->GetPointData()->AddArray(lineCorrect);
	
	cout << "Assigning length to each source vertex ..." << endl;
	vtkDataArray* seedIds = streamLines->GetCellData()->GetScalars("SeedIds");
	if (seedIds) {
		// line clipping
		vtkPoints* outputPoints = vtkPoints::New();
		vtkCellArray* outputCells = vtkCellArray::New();
		
		/// construct a tree locator
		vtkModifiedBSPTree* tree = vtkModifiedBSPTree::New();
		tree->SetDataSet(destinationSurface);
		tree->BuildLocator();
		
		vtkDoubleArray* lengthArray = vtkDoubleArray::New();
		lengthArray->SetName("Length");
		
		vtkIntArray* pointIds = vtkIntArray::New();
		pointIds->SetName("PointIds");
		
		cout << "# of cells: " << nCells << endl;

		int noLines = 0;
		for (int i = 0; i < nCells; i++) {
			int pid = seedIds->GetTuple1(i);
			double length = 0;
			if (pid > -1) {
				vtkCell* line = streamLines->GetCell(i);
				/// - Assume that a line starts from a point on the input mesh and must meet at the opposite surface of the starting point.
                bool lineAdded = performLineClipping(streamLines, tree, /*i,*/ line, outputPoints, outputCells, length);
				
				if (lineAdded) {
					pointIds->InsertNextValue(pid);
					lengthArray->InsertNextValue(length);
					streamLineLengthPerPoint->SetValue(pid, length);
					lineCorrect->SetValue(pid, 1);
				} else {
					pointIds->InsertNextValue(pid);
					lengthArray->InsertNextValue(length);
					streamLineLengthPerPoint->SetValue(pid, length);
					lineCorrect->SetValue(pid, 2);
					noLines++;
				}
			}
		}
		
		cout << "# of clipping failure: " << noLines << endl;
		
		vtkPolyData* outputStreamLines = vtkPolyData::New();
		outputStreamLines->SetPoints(outputPoints);
		outputStreamLines->SetLines(outputCells);
		outputStreamLines->GetCellData()->AddArray(pointIds);
		outputStreamLines->GetCellData()->AddArray(lengthArray);
	
		
		vtkCleanPolyData* cleaner = vtkCleanPolyData::New();
		cleaner->SetInputData(outputStreamLines);
		cleaner->Update();
		
		return cleaner->GetOutput();
	} else {
		cout << "Can't find SeedIds" << endl;
		return NULL;
	}
}

void SurfaceCorrespondence::performStreamTracer() {
    if (m_laplaceField == NULL || m_whiteMatterSurface == NULL) {
        cout << "input vector field or seed points is null!" << endl;
        return;
    }
    
    if (m_whiteMatterSurface == NULL) {
        cout << "trace destination surface is null" << endl;
        return;
    }
    
	// set active velocity field
    m_laplaceField->GetPointData()->SetActiveVectors("LaplacianGradientNorm");
	
	/// - Converting the input points to the image coordinate
    vtkPoints* points = m_whiteMatterSurface->GetPoints();
	cout << "# of seed points: " << points->GetNumberOfPoints() << endl;
	
	/// StreamTracer should have a point-wise gradient field
	/// - Set up tracer (Use RK45, both direction, initial step 0.05, maximum propagation 500
	vtkStreamTracer* tracer = vtkStreamTracer::New();
    tracer->SetInputData(m_laplaceField);
    tracer->SetSourceData(m_whiteMatterSurface);
	tracer->SetComputeVorticity(false);

	tracer->SetIntegrationDirectionToForward();
	cout << "Forward Integration" << endl;

	tracer->SetInterpolatorTypeToCellLocator();
	tracer->SetMaximumPropagation(5000);
    tracer->SetInitialIntegrationStep(0.01);
    tracer->SetIntegratorTypeToRungeKutta45();

    cout << "Integration Direction: " << tracer->GetIntegrationDirection() << endl;
    cout << "Initial Integration Step: " << tracer->GetInitialIntegrationStep() << endl;
    cout << "Maximum Integration Step: " << tracer->GetMaximumIntegrationStep() << endl;
    cout << "Minimum Integration Step: " << tracer->GetMinimumIntegrationStep() << endl;
    cout << "Maximum Error: " << tracer->GetMaximumError() << endl;
    cout << "IntegratorType: " << tracer->GetIntegratorType() << endl;

    tracer->Update();
	
    vtkPolyData* streamLines = tracer->GetOutput();
	
    m_streams = performStreamTracerPostProcessing(streamLines, m_whiteMatterSurface, m_whiteMatterSurface);
}

void SurfaceCorrespondence::findNeighborPoints(vtkCell *cell, vtkIdType pid, set<vtkIdType> &nbrs){
    for (/*size_t*/int j = 0; j < cell->GetNumberOfPoints(); j++) {
        vtkIdType cellPt = cell->GetPointId(j);
        if (pid != cellPt) {
            nbrs.insert(cellPt);
        }
    }
}

void SurfaceCorrespondence::interpolateBrokenPoints(vtkPolyData* surf, vtkPoints* warpedPoints, vtkDataArray* seedIds){
    // identify broken points
    vector<vtkIdType> brokenPoints;
    vtkIdType z = 0;
    for (/*size_t*/int j = 0; j < seedIds->GetNumberOfTuples(); j++,z++) {
        vtkIdType y = seedIds->GetTuple1(j);
        while (z < y) {
            brokenPoints.push_back(z++);
        }
    }

    // find neighbors and compute interpolatead points
    vtkNew<vtkIdList> cellIds;
    set<vtkIdType> nbrs;
    for (size_t j = 0; j < brokenPoints.size(); j++) {
        vtkIdType pid = brokenPoints[j];
        cellIds->Reset();
        surf->GetPointCells(pid, cellIds.GetPointer());
        nbrs.clear();
        // find neighbor points
        for (/*size_t*/int k = 0; k < cellIds->GetNumberOfIds(); k++) {
            vtkCell* cell = surf->GetCell(k);
            findNeighborPoints(cell, pid, nbrs);
        }
        // average neighbor points
        double p[3] = {0,}, q[3] = {0,};
        set<vtkIdType>::iterator it = nbrs.begin();
        for (; it != nbrs.end(); it++) {
            if (find(brokenPoints.begin(), brokenPoints.end(), *it) == brokenPoints.end()) {
                warpedPoints->GetPoint(*it, q);
                vtkMath::Add(p, q, p);
            } else {
                cout << "broken neighbor!! " << pid << "," << *it << endl;
            }
        }
        p[0]/=nbrs.size();
        p[1]/=nbrs.size();
        p[2]/=nbrs.size();
        warpedPoints->SetPoint(pid, p);
    }
}

void SurfaceCorrespondence::run(){
    string outputGrid = m_output_dir + m_prefix + "_grid.vts";
    string outputField = m_output_dir + m_prefix + "_field.vts";
    string outputStream = m_output_dir + m_prefix + "_stream.vtp";
    string outputMesh = m_output_dir + m_prefix + "_warpedMesh.vtp";

    cout << "Output grid: " << outputGrid << endl;
    cout << "Output laplacian field: " << outputField << endl;
    cout << "Output streamlines: " << outputStream << endl;
    cout << "Output warped mesh: " << outputMesh << endl;

    // create uniform grid for a FDM model
    createGrid();

    if (m_writeGridFile)
    {
        m_vio.writeFile(outputGrid, m_laplaceField);
    }

    // compute laplace map
    computeLaplacePDE();

    if (m_writeLaplaceFieldFile)
    {
        m_vio.writeFile(outputField, m_laplaceField);
    }

    performStreamTracer();
    if (m_writeStreamFile)
    {
        m_vio.writeFile(outputStream, m_streams);
    }
}
