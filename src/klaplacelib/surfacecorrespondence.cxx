#include <iostream>
#include <map>
#include <vector>
#include <set>

using namespace std;

#include "surfacecorrespondence.h"
#include "vtkio.h"
#include "geometry.h"
#include "gridcreate.h"
#include "boundarycheck.h"

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

SurfaceCorrespondance::SurfaceCorrespondance(string inputObj1, string inputObj2, string prefix, int dims):
    m_inputObj1(inputObj1),
    m_inputObj2(inputObj2),
    m_prefix(prefix),
    m_dims(dims)
{
    /*
    if (m_inputObj1 == "" || m_inputObj2 == "") {
        cout << "-surfaceCorrespondence option needs two inputs" << endl;
    }
    */
}

vtkDataSet* SurfaceCorrespondance::createGrid(vtkPolyData* osurf, vtkPolyData* isurf, const int dims, size_t& insideCountOut) {
	GridCreate gc(osurf->GetBounds(), dims);
	
	vtkNew<vtkImageData> gim;
	vtkNew<vtkImageData> wim;
	
	vtkStructuredGrid* goim = gc.createStencil(gim.GetPointer(), osurf);
	vtkStructuredGrid* woim = gc.createStencil(wim.GetPointer(), isurf);

	//visualiseTwoDataSets(goim,woim);
	
	BoundaryCheck bc;
	insideCountOut = bc.subtract(goim, woim);
	bc.checkSurface(goim, isurf, osurf);
	woim->Delete();
	
	return goim;
}

void SurfaceCorrespondance::runFillGrid(StringVector& args, int dims) {
// create a structured grid with the size of input
// convert the grid to polydata
// create the intersection between the grid and the polydata
	string outputFile = args[2];
	
	vtkIO vio;    
	vtkPolyData* osurf = vio.readFile(args[0]);
	vtkPolyData* isurf = vio.readFile(args[1]);    
	
	size_t insideCountOut = 0;
	vtkDataSet* grid = createGrid(osurf, isurf, dims, insideCountOut);
	vio.writeFile(outputFile, grid);
	
	cout << "Inside Voxels: " << insideCountOut << endl;
	
}

void SurfaceCorrespondance::computeLaplacePDE(vtkDataSet* data, const double low, const double high, const int nIters, const double dt, vtkPolyData* surfaceData /*= NULL*/) {
// Compute Laplace PDE based on the adjacency list and border
	if (data == NULL) {
		cout << "Data input is NULL" << endl;
		return;
	}
	
	class LaplaceGrid {
	public:
		double low;
		double high;
		double dt;
		vtkDataSet* dataSet;
		vtkPolyData* samplePoints;
		
		vector<vtkIdType> solutionDomain;
		vtkIntArray* boundaryCond;
		vtkPolyData* boundarySurface;
		
		vtkDoubleArray* solution;
		vtkDoubleArray* tmpSolution;
		vtkDataArray* laplaceGradient;
		vtkDoubleArray* laplaceGradientNormals;
		
		
		Geometry geom;
		Geometry::NeighborList nbrs;
		
		
		
		LaplaceGrid(double l, double h, double d, vtkDataSet* ds, vtkPolyData* pd= NULL): low(l), high(h), dt(d), dataSet(ds), boundarySurface(pd) {
			cout << "geometry edge extraction ... " << flush;
			geom.extractNeighbors(ds, nbrs);
			cout << " done " << endl;
			
			// check boundary points
			boundaryCond = vtkIntArray::SafeDownCast(ds->GetPointData()->GetArray("SampledValue"));
			if (boundaryCond == NULL) {
				throw runtime_error("No scalar values for BoundaryPoints");
			}
			
			initializeSolution();
		}
		
		void initializeSolution() {
			cout << "initializing solution grid ... " << flush;
			// low-value 2
			// high-value 1
			solution = vtkDoubleArray::New();
			solution->SetName("LaplacianSolution");
			solution->SetNumberOfComponents(1);
			solution->SetNumberOfTuples(boundaryCond->GetNumberOfTuples());
			solution->FillComponent(0, 0);
			
			tmpSolution = vtkDoubleArray::New();
			tmpSolution->SetName("LaplacianSolution");
			tmpSolution->SetNumberOfComponents(1);
			tmpSolution->SetNumberOfTuples(boundaryCond->GetNumberOfTuples());
			tmpSolution->FillComponent(0, 0);
			
			const size_t nPts = boundaryCond->GetNumberOfTuples();
			for (size_t j = 0; j < nPts; j++) {
				int domain = boundaryCond->GetValue(j);
				double uValue = 0;
				if (domain == 700) {
					// high
					uValue = high;
				} else if (domain == 300){
					// low
					uValue = low;
				} else if (domain == 1) {
					uValue = 0;
					solutionDomain.push_back(j);
				}
				solution->SetValue(j, uValue);
				tmpSolution->SetValue(j, uValue);
			}
			cout << "# of points: " << solutionDomain.size() << endl;
		}
		
		void computeStep() {
			const size_t nPts = solutionDomain.size();
			for (size_t j = 0; j < nPts; j++) {
				vtkIdType centerId = solutionDomain[j];
				Geometry::Neighbors& edgeMap = nbrs[centerId];
				Geometry::Neighbors::iterator iter = edgeMap.begin();
				
				double u = 0;
				double nNbrs = 0;
				for (; iter != edgeMap.end(); iter++) {
					const double du = solution->GetValue(*iter);
					u += du;
					nNbrs ++;

					//                    cout << iter->second.axisAligned << endl;
				}
				u = u / nNbrs;
				tmpSolution->SetValue(centerId, u);
			}
			
			vtkDoubleArray* swapTmp = tmpSolution;
			tmpSolution = solution;
			solution = swapTmp;
//			memcpy(solution->WritePointer(0, nPts), tmpSolution->GetVoidPointer(0), sizeof(double) * nTuples);
//			solution->DeepCopy(tmpSolution);
		}
		
		
		void computeNormals(vtkDataSet* data) {
			/*
			 vtkNew<vtkCellDerivatives> deriv;
			 deriv->SetInput(data);
			 deriv->SetVectorModeToComputeGradient();
			 deriv->Update();
			 vtkDataSet* derivOut = deriv->GetOutput();
			 derivOut->GetCellData()->SetActiveVectors("ScalarGradient");
			 vtkDataArray* scalarGradient = deriv->GetOutput()->GetCellData()->GetArray("ScalarGradient");
			 scalarGradient->SetName("LaplacianGradient");
			 */
			
			vtkNew<vtkGradientFilter> gradFilter;
			gradFilter->SetInputData(data);
			gradFilter->SetInputScalars(vtkDataSet::FIELD_ASSOCIATION_POINTS, "LaplacianSolution");
			gradFilter->SetResultArrayName("LaplacianGradient");
			gradFilter->Update();
			laplaceGradient = gradFilter->GetOutput()->GetPointData()->GetArray("LaplacianGradient");
			laplaceGradient->Register(NULL);
			
			laplaceGradientNormals = vtkDoubleArray::New();
			laplaceGradientNormals->SetName("LaplacianGradientNorm");
			laplaceGradientNormals->SetNumberOfComponents(3);
			laplaceGradientNormals->SetNumberOfTuples(laplaceGradient->GetNumberOfTuples());
			
			const size_t nPts = laplaceGradientNormals->GetNumberOfTuples();
			for (size_t j = 0; j < nPts; j++) {
				double* vec = laplaceGradient->GetTuple3(j);
				double norm = vtkMath::Norm(vec);
				if (norm > 1e-10) {
					laplaceGradientNormals->SetTuple3(j, vec[0]/norm, vec[1]/norm, vec[2]/norm);
				} else {
					laplaceGradientNormals->SetTuple3(j, 0, 0, 0);

				}
			}
			
			data->GetPointData()->AddArray(laplaceGradient);
			data->GetPointData()->SetVectors(laplaceGradientNormals);
		}
		
		void computeExteriorNormals(vtkPolyData* boundarySurface, const double radius = .1) {
			if (boundarySurface == NULL) {
				return;
			}
			vtkNew<vtkPolyDataNormals> normalsFilter;
			normalsFilter->SetInputData(boundarySurface);
			normalsFilter->ComputeCellNormalsOn();
			normalsFilter->ComputePointNormalsOn();
			normalsFilter->Update();
			vtkFloatArray* cellNormals = vtkFloatArray::SafeDownCast(normalsFilter->GetOutput()->GetCellData()->GetNormals());
			
			vtkNew<vtkCellLocator> cloc;
			cloc->SetDataSet(boundarySurface);
			cloc->AutomaticOn();
			cloc->BuildLocator();
			
			dataSet->GetPointData()->SetActiveScalars("SampledValue");
			
			vtkNew<vtkThresholdPoints> threshold;
			threshold->SetInputData(dataSet);
			threshold->ThresholdByUpper(250);
			threshold->Update();
			vtkDataSet* inoutBoundary = threshold->GetOutput();
			vtkIntArray* inoutBoundaryCond = vtkIntArray::SafeDownCast(inoutBoundary->GetPointData()->GetArray("SampledValue"));
			
			
			vtkNew<vtkPointLocator> ploc;
			ploc->SetDataSet(inoutBoundary);
			ploc->AutomaticOn();
			ploc->BuildLocator();
			
			const size_t nPts = dataSet->GetNumberOfPoints();
			for (size_t j = 0; j < nPts; j++) {
				int domain = boundaryCond->GetValue(j);
				if (domain == 700 || domain == 300 || domain == 0) {
					double x[3] = { 0, }, closestPoint[3] = { 0, };
					vtkIdType cellId = -1;
					int subId = 0;
					double dist2 = -1;
					dataSet->GetPoint(j, x);
					vtkNew<vtkGenericCell> closestCell;
					cloc->FindClosestPointWithinRadius(x, radius, closestPoint, cellId, subId, dist2);
					
					float cellNormal[3];
					cellNormals->GetTypedTuple(cellId, cellNormal);
					cellNormal[0] = 0;
					vtkMath::Normalize(cellNormal);
					
					if (domain == 0) {
						vtkIdType xId = ploc->FindClosestPoint(x);
						domain = inoutBoundaryCond->GetValue(xId);
						assert(domain == 300 || domain == 700);
					}
					
					
					if (domain == 300) {
						laplaceGradientNormals->SetTuple3(j, -cellNormal[0], -cellNormal[1], -cellNormal[2]);
					} else {
						laplaceGradientNormals->SetTuple3(j, cellNormal[0], cellNormal[1], cellNormal[2]);
					}
				}
			}
		}
	};
	
	
	LaplaceGrid grid(low, high, dt, data, surfaceData);
	
	clock_t t1 = clock();
	
	// main iteration loop
    for (/*size_t*/int i = 1; i <= nIters; i++) {
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
	data->GetPointData()->AddArray(grid.solution);
	grid.computeNormals(data);
//	grid.computeExteriorNormals(surfaceData);
}

bool SurfaceCorrespondance::performLineClipping(vtkPolyData* streamLines, vtkModifiedBSPTree* tree, /*int lineId,*/ vtkCell* lineToClip, vtkPoints* outputPoints, vtkCellArray* outputLines, double &length) {
	
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

vtkPolyData* SurfaceCorrespondance::performStreamTracerPostProcessing(vtkPolyData* streamLines, vtkPolyData* seedPoints, vtkPolyData* destinationSurface) {
	
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

vtkPolyData* SurfaceCorrespondance::performStreamTracer(vtkDataSet* inputData, vtkPolyData* inputSeedPoints, vtkPolyData* destSurf, bool zRotate /*= false*/) {
    if (inputData == NULL || inputSeedPoints == NULL) {
        cout << "input vector field or seed points is null!" << endl;
        return NULL;
    }
    
    if (destSurf == NULL) {
        cout << "trace destination surface is null" << endl;
        return NULL;
    }
    
	// set active velocity field
	inputData->GetPointData()->SetActiveVectors("LaplacianGradientNorm");
	
	/// - Converting the input points to the image coordinate
	vtkPoints* points = inputSeedPoints->GetPoints();
	cout << "# of seed points: " << points->GetNumberOfPoints() << endl;
	const int nInputPoints = inputSeedPoints->GetNumberOfPoints();
	if (zRotate) {
		for (int i = 0; i < nInputPoints; i++) {
			double p[3];
			points->GetPoint(i, p);
			// FixMe: Do not use a specific scaling factor
			if (zRotate) {
				p[0] = -p[0];
				p[1] = -p[1];
				p[2] = p[2];
			}
			points->SetPoint(i, p);
		}
		inputSeedPoints->SetPoints(points);
	}
	
	
	/// StreamTracer should have a point-wise gradient field
	/// - Set up tracer (Use RK45, both direction, initial step 0.05, maximum propagation 500
	vtkStreamTracer* tracer = vtkStreamTracer::New();
	tracer->SetInputData(inputData);
	tracer->SetSourceData(inputSeedPoints);
	tracer->SetComputeVorticity(false);

	tracer->SetIntegrationDirectionToForward();
	cout << "Forward Integration" << endl;

//    tracer->SetInterpolatorTypeToDataSetPointLocator();
	tracer->SetInterpolatorTypeToCellLocator();
	tracer->SetMaximumPropagation(5000);
	tracer->SetInitialIntegrationStep(0.01);
//    tracer->SetMaximumIntegrationStep(0.1);
    tracer->SetIntegratorTypeToRungeKutta45();
//    tracer->SetIntegratorTypeToRungeKutta2();

    cout << "Integration Direction: " << tracer->GetIntegrationDirection() << endl;
    cout << "Initial Integration Step: " << tracer->GetInitialIntegrationStep() << endl;
    cout << "Maximum Integration Step: " << tracer->GetMaximumIntegrationStep() << endl;
    cout << "Minimum Integration Step: " << tracer->GetMinimumIntegrationStep() << endl;
    cout << "Maximum Error: " << tracer->GetMaximumError() << endl;
    cout << "IntegratorType: " << tracer->GetIntegratorType() << endl;

    
    tracer->Update();

	
	vtkPolyData* streamLines = tracer->GetOutput();
//	streamLines->Print(cout);
	
//	vio.writeFile("streamlines.vtp", streamLines);
	
	return performStreamTracerPostProcessing(streamLines, inputSeedPoints, destSurf);
}

void findNeighborPoints(vtkCell* cell, vtkIdType pid, set<vtkIdType>& nbrs) {
    for (/*size_t*/int j = 0; j < cell->GetNumberOfPoints(); j++) {
        vtkIdType cellPt = cell->GetPointId(j);
        if (pid != cellPt) {
            nbrs.insert(cellPt);
        }
    }
}

void SurfaceCorrespondance::runPrintTraceCorrespondence(string inputMeshName, string inputStreamName, string outputWarpedMeshName, vtkPolyData* srcmesh /*= NULL*/) {
	vtkIO vio;
	
	if (srcmesh == NULL) {
		srcmesh = vio.readFile(inputMeshName);
	}
	
	vtkNew<vtkPolyData> warpedMesh;
	warpedMesh->DeepCopy(srcmesh);
	
	srcmesh->ComputeBounds();
	
	double center[3];
	srcmesh->GetCenter(center);
	
	vtkDataSet* strmesh = vio.readDataFile(inputStreamName);
	
	vtkNew<vtkDoubleArray> pointArr;
	pointArr->SetName("SourcePoints");
	pointArr->SetNumberOfComponents(3);
	pointArr->SetNumberOfTuples(srcmesh->GetNumberOfPoints());
	
	vtkNew<vtkDoubleArray> sphrCoord;
	sphrCoord->SetName("SphericalCoordinates");
	sphrCoord->SetNumberOfComponents(3);
	sphrCoord->SetNumberOfTuples(srcmesh->GetNumberOfPoints());
	
	vtkNew<vtkSphericalTransform> sphTxf;
	sphTxf->Inverse();
	
	
	vtkNew<vtkDoubleArray> destPointArr;
	destPointArr->SetName("DestinationPoints");
	destPointArr->SetNumberOfComponents(3);
	destPointArr->SetNumberOfTuples(srcmesh->GetNumberOfPoints());
	
	vtkNew<vtkDoubleArray> sphereRadiusArr;
	sphereRadiusArr->SetName("SphereRadius");
	sphereRadiusArr->SetNumberOfComponents(1);
	sphereRadiusArr->SetNumberOfTuples(srcmesh->GetNumberOfPoints());
	
	vtkNew<vtkPoints> warpedPoints;
	warpedPoints->DeepCopy(srcmesh->GetPoints());
	
	vtkNew<vtkPointLocator> ploc;
	ploc->SetDataSet(srcmesh);
	ploc->SetTolerance(0);
	ploc->BuildLocator();
	
	
	
	const size_t nCells = strmesh->GetNumberOfCells();
	vtkDataArray* seedIds = strmesh->GetCellData()->GetArray("PointIds");
	
	
	for (size_t j = 0; j < nCells; j++) {
		vtkCell* cell = strmesh->GetCell(j);
		const size_t nPts = cell->GetNumberOfPoints();
		if (nPts < 2) {
			continue;
		}
		vtkIdType s = cell->GetPointId(0);
		vtkIdType e = cell->GetPointId(nPts-1);
		
        double qs[3], qe[3],/* pj[3], spj[3],*/ npj[3];
		strmesh->GetPoint(s, qs);
		strmesh->GetPoint(e, qe);
		
		vtkIdType seedId = (vtkIdType) seedIds->GetTuple1(j);
		warpedPoints->SetPoint(seedId, qe);
		
		vtkMath::Subtract(qe, center, npj);
		double warpedPointNorm = vtkMath::Norm(npj);
		
        sphereRadiusArr->SetValue(j, warpedPointNorm);

//		cout << "cell " << j << ": " << nPts << ", " << s << " => " << e << endl;
//		cout << "cell " << j << ": " << qe[0] << "," << qe[1] << "," << qe[2] << endl;

//
//		srcmesh->GetPoint(seedId, pj);
//		pointArr->SetTupleValue(seedId, pj);
//
//		vtkMath::Subtract(pj, sphereCenter, npj);
//		sphTxf->TransformPoint(npj, spj);
//		sphrCoord->SetTupleValue(seedId, spj);
//
//		destPointArr->SetTupleValue(seedId, qe);

	}
	
	srcmesh->GetPointData()->AddArray(sphereRadiusArr.GetPointer());

    struct InterpolateBrokenPoints {
		InterpolateBrokenPoints(vtkPolyData* surf, vtkPoints* warpedPoints, vtkDataArray* seedIds) {
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

	};
	
	warpedMesh->SetPoints(warpedPoints.GetPointer());
	InterpolateBrokenPoints(warpedMesh.GetPointer(), warpedPoints.GetPointer(), seedIds);
	
	//	warpedMesh->Print(cout);
//	warpedMesh->GetPointData()->SetVectors(pointArr.GetPointer());
//	warpedMesh->GetPointData()->AddArray(sphrCoord.GetPointer());
	vio.writeFile(outputWarpedMeshName, warpedMesh.GetPointer());
	
//	if (args.size() > 3) {
//		srcmesh->GetPointData()->SetVectors(destPointArr.GetPointer());
//		srcmesh->GetPointData()->AddArray(sphrCoord.GetPointer());
//		vio.writeFile(args[3], srcmesh);
//	}
}

void SurfaceCorrespondance::run(){
    vtkIO vio;

    string outputGrid = m_prefix + "_grid.vts";
    string outputField = m_prefix + "_field.vts";
    string outputStream = m_prefix + "_stream.vtp";
    string outputMesh = m_prefix + "_warpedMesh.vtp";
    //string outputObj = m_prefix + "_object.vtp";

    cout << "Output grid: " << outputGrid << endl;
    cout << "Output laplacian field: " << outputField << endl;
    cout << "Output streamlines: " << outputStream << endl;
    cout << "Output warped mesh: " << outputMesh << endl;


    // create uniform grid for a FDM model
    StringVector fillGridArgs;
    fillGridArgs.push_back(m_inputObj2);
    fillGridArgs.push_back(m_inputObj1);
    fillGridArgs.push_back(outputGrid);

    runFillGrid(fillGridArgs,m_dims);


    // compute laplace map
    vtkDataSet* laplaceField = NULL;
    laplaceField = vio.readDataFile(outputGrid);

    //visualiseDataSet(laplaceField);

    const double dt = 0.125;
    //const int numIter = 10000;
    const int numIter = 1000;
    computeLaplacePDE(laplaceField, 0, 10000, numIter, dt);
    vio.writeFile(outputField, laplaceField);

    /*
    vtkPolyData* inputData = vio.readFile(m_inputObj1);
    vtkPolyData* inputData2 = vio.readFile(m_inputObj2);

    if (inputData == NULL) {
        cout << m_inputObj1 << " is null" << endl;
        return 1;
    }
    if (inputData2 == NULL) {
        cout << m_inputObj2 << " is null" << endl;
        return 1;
    }

    vtkPolyData* streams = performStreamTracer(laplaceField, inputData, inputData2);
    vio.writeFile(outputStream, streams);
    runPrintTraceCorrespondence(m_inputObj1, outputStream, outputMesh, inputData);


    vio.writeFile(outputObj, inputData);
    */
}

int main(int argc, char* argv[]) {
    //cout<<"inputs: "<<endl<<inputObj1<<endl<<inputObj2<<endl<<prefix<<endl<<dims<<endl<<endl;

    if (argc != 5) {
        cout << "-surfaceCorrespondence option needs two inputs, one prefix and the number of dims" << endl;
    }

    string inputObj1 = argv[1];
    string inputObj2 = argv[2];
    string prefix = argv[3];
    int dims = atoi(argv[4]);

    SurfaceCorrespondance sCorr(inputObj1,inputObj2,prefix,dims);

    sCorr.run();
}
