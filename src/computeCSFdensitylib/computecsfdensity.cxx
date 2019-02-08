#include "computecsfdensity.h"
#include "surfacecorrespondence.h"

#include <iostream>
#include <sys/types.h>
#include <sys/stat.h>
#include <cmath>        // abs
#include <limits>       // std::numeric_limits
#include <ctime>
#include <algorithm>

#include "vtkAppendFilter.h"
#include "vtkCellData.h"
#include "vtkDecimatePro.h"
#include "vtkDoubleArray.h"
#include "vtkGradientFilter.h"
#include "vtkMarchingCubes.h"
#include "vtkPolyDataWriter.h"
#include "vtkPointData.h"
#include "vtkSmoothPolyDataFilter.h"
#include "vtkTransform.h"
#include "vtkTransformPolyDataFilter.h"
#include "vtkUnstructuredGrid.h"

#include "itkBinaryBallStructuringElement.h"
#include "itkBinaryMorphologicalClosingImageFilter.h"
#include "itkBinaryThresholdImageFilter.h"
#include "itkCastImageFilter.h"
#include "itkImageFileReader.h"
#include "itkImageFileWriter.h"
#include "itkImageRegionIterator.h"
#include "itkImageToVTKImageFilter.h"
#include "itkLinearInterpolateImageFunction.h"
#include "itkRescaleIntensityImageFilter.h"

#include "vtkImageWriter.h"

using namespace std;

ComputeCSFdensity::ComputeCSFdensity(string whiteMatterSurface_fileName, string segFile, string csfPropFile, string prefix, string output_dir){
    // Creating output directory if necessary
    int dirstatus = setOutputLocation(output_dir);
    if (dirstatus == EXIT_FAILURE){
        exit(1);
    }

    // Reading vtk surfaces
    m_whiteMatterSurface = m_vio.readFile(whiteMatterSurface_fileName);


    // Reading itk images (segmentation  and probability map)
    typedef itk::ImageFileReader<m_ucImageType> ucReaderType;
    ucReaderType::Pointer readerucSeg = ucReaderType::New();
    readerucSeg->SetFileName(segFile);
    readerucSeg->Update();
    m_ucseg = readerucSeg->GetOutput();

    typedef itk::ImageFileReader<m_dImageType> dReaderType;
    dReaderType::Pointer readerdSeg = dReaderType::New();
    readerdSeg->SetFileName(segFile);
    readerdSeg->Update();
    m_dseg = readerdSeg->GetOutput();

    dReaderType::Pointer readerCSFprop = dReaderType::New();
    readerCSFprop->SetFileName(csfPropFile);
    readerCSFprop->Update();
    m_CSFprop = readerCSFprop->GetOutput();

    // Other members setting
    m_WM_relFilename = relativePath(whiteMatterSurface_fileName);
    m_GM_relFilename = relativePath(greyMatterSurface_fileName);

    m_prefix = prefix;
}

int ComputeCSFdensity::setOutputLocation(string dirname){
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

vector<string> ComputeCSFdensity::splitExt(string filename){
    size_t pos = filename.find_last_of('.');
    string ext = "";
    if (pos != string::npos){
        ext = filename.substr(pos);
        filename.resize(pos);
    }
    vector<string> ret;
    ret.push_back(filename);
    ret.push_back(ext);
    return ret;
}

string ComputeCSFdensity::relativePath(string path){
    string relPath;
    size_t pos = path.find_last_of('/');
    if (pos != string::npos){
        relPath = path.erase(0,pos+1);
    }
    return relPath;
}

void ComputeCSFdensity::createOuterImage(int closingradius, int dilationradius, bool reverse){
    cout<<"Creating outer image ..."<<endl;

    m_ucImageType::SpacingType spacing = m_ucseg->GetSpacing();
    closingradius = closingradius/spacing[0];

    typedef itk::BinaryThresholdImageFilter <m_ucImageType, m_ucImageType> BinaryThresholdImageFilterType;
    BinaryThresholdImageFilterType::Pointer thresholdFilter = BinaryThresholdImageFilterType::New();
    thresholdFilter->SetInput(m_ucseg);
    thresholdFilter->SetLowerThreshold(1);
    thresholdFilter->SetUpperThreshold(4);
    thresholdFilter->SetInsideValue(1);
    thresholdFilter->SetOutsideValue(0);
    thresholdFilter->Update();

    m_outerImage = thresholdFilter->GetOutput();

    m_ucImageType::SizeType regionSize;
    regionSize[0] = int(m_outerImage->GetLargestPossibleRegion().GetSize()[0]/2);
    regionSize[1] = m_outerImage->GetLargestPossibleRegion().GetSize()[1];
    regionSize[2] = m_outerImage->GetLargestPossibleRegion().GetSize()[2];

    m_ucImageType::IndexType regionIndex;
    regionIndex[0] = 0;
    if (reverse){
        regionIndex[0] = m_outerImage->GetLargestPossibleRegion().GetSize()[0]/2;
    }
    regionIndex[1] = 0;
    regionIndex[2] = 0;

    m_ucImageType::RegionType region;
    region.SetSize(regionSize);
    region.SetIndex(regionIndex);

    itk::ImageRegionIterator<m_ucImageType> imageIterator(m_outerImage,region);

    while(!imageIterator.IsAtEnd()){
        imageIterator.Set(0);
        ++imageIterator;
    }

    typedef itk::BinaryBallStructuringElement<m_ucImageType::PixelType,3> StructuringElementType;
    StructuringElementType structuringElement;
    structuringElement.SetRadius(closingradius);
    structuringElement.CreateStructuringElement();

    typedef itk::RescaleIntensityImageFilter<m_ucImageType, m_ucImageType > RescaleFilterType;
    RescaleFilterType::Pointer rescaleFilter = RescaleFilterType::New();
    rescaleFilter->SetInput(m_outerImage);
    rescaleFilter->SetOutputMinimum(0);
    rescaleFilter->SetOutputMaximum(255);
    rescaleFilter->Update();

    typedef itk::BinaryMorphologicalClosingImageFilter <m_ucImageType, m_ucImageType, StructuringElementType> BinaryMorphologicalClosingImageFilterType;
    BinaryMorphologicalClosingImageFilterType::Pointer closingFilter = BinaryMorphologicalClosingImageFilterType::New();
    closingFilter->SetInput(rescaleFilter->GetOutput());
    closingFilter->SetKernel(structuringElement);
    closingFilter->Update();

    dilationradius = dilationradius/spacing[0];

    structuringElement.SetRadius(dilationradius);
    structuringElement.CreateStructuringElement();

    typedef itk::BinaryDilateImageFilter <m_ucImageType, m_ucImageType, StructuringElementType> BinaryDilateImageFilterType;
    BinaryDilateImageFilterType::Pointer dilateFilter = BinaryDilateImageFilterType::New();
    dilateFilter->SetInput(closingFilter->GetOutput());
    dilateFilter->SetKernel(structuringElement);
    dilateFilter->Update();

    rescaleFilter->SetInput(dilateFilter->GetOutput());
    rescaleFilter->SetOutputMinimum(0);
    rescaleFilter->SetOutputMaximum(1);
    rescaleFilter->Update();

    m_outerImage = rescaleFilter->GetOutput();

    if (m_writeOuterImage){
        string filename = m_output_dir + m_prefix + "_GM_Dilated.nrrd";
        typedef itk::ImageFileWriter< m_ucImageType >  WriterType;
        WriterType::Pointer writer = WriterType::New();
        writer->SetFileName(filename);
        writer->SetInput(m_outerImage);
        writer->UseCompressionOn ();
        try
          {
          writer->Update();
          cout << "Write " << filename << " done ..." << endl;
          }
        catch( itk::ExceptionObject & e )
          {
          cerr << "Error: " << e << endl;
          }
    }

    cout<<"Outer image created"<<endl;
}

void ComputeCSFdensity::createOuterSurface(int nbIterSmoothing){
    cout<<"Creating outer surface ..."<<endl;
    typedef itk::ImageToVTKImageFilter<m_ucImageType> itkVtkConverter;
    itkVtkConverter::Pointer conv = itkVtkConverter::New();
    conv->SetInput(m_outerImage);
    conv->Update();

    vtkImageWriter *imWriter = vtkImageWriter::New();
    imWriter->SetInputData(conv->GetOutput());
    imWriter->SetFileName("itkConvImg_alm.mhd");
    imWriter->Update();

    vtkSmartPointer<vtkMarchingCubes> outputsurface = vtkSmartPointer<vtkMarchingCubes>::New();
    outputsurface->SetInputData(conv->GetOutput());
    outputsurface->ComputeNormalsOn();
    outputsurface->SetValue(0, 1);
    outputsurface->Update();
    cout << "Marching Cube finished...." << endl;

    cout << "Before decimation" << endl << "------------" << endl;
    cout << "There are " << outputsurface->GetOutput()->GetNumberOfPoints() << " points." << endl;
    cout << "There are " << outputsurface->GetOutput()->GetNumberOfPolys() << " polygons." << endl;


    vtkSmartPointer<vtkDecimatePro> decimate = vtkSmartPointer<vtkDecimatePro>::New();
    decimate->SetInputData(outputsurface->GetOutput());
    decimate->SetTargetReduction(.1); //10% reduction (if there was 100 triangles, now there will be 90)
    decimate->Update();

    cout << "After decimation" << endl << "------------" << endl;
    cout << "There are " << decimate->GetOutput()->GetNumberOfPoints() << " points." << endl;
    cout << "There are " << decimate->GetOutput()->GetNumberOfPolys() << " polygons." << endl;

    vtkSmartPointer<vtkSmoothPolyDataFilter> smoothFilter = vtkSmartPointer<vtkSmoothPolyDataFilter>::New();
    smoothFilter->SetInputConnection( decimate->GetOutputPort());
    smoothFilter->SetNumberOfIterations(nbIterSmoothing);
    smoothFilter->SetRelaxationFactor(0.5);
    smoothFilter->FeatureEdgeSmoothingOff();
    smoothFilter->BoundarySmoothingOn();
    smoothFilter->Update();
    cout << "VTK Smoothing mesh finished...." << endl;
    m_outerSurface = smoothFilter->GetOutput();

    // Get the Surface filename from the command line

    if (m_writeOuterSurface){
        string outputSurfaceFilename = m_output_dir + m_prefix + "_GM_Outer_MC.vtk";
        m_vio.writeFile(outputSurfaceFilename,m_outerSurface);
//        vtkSmartPointer<vtkPolyDataWriter> writer = vtkSmartPointer<vtkPolyDataWriter>::New();
//        writer->SetFileName(outputSurfaceFilename.c_str());
//        writer->SetInputData(m_outerSurface);
//        writer->SetFileTypeToASCII();
//        writer->Write();
    }

    cout << "Input surface has " << m_outerSurface->GetNumberOfPoints() << " points." << endl;

    cout<<"Outer image created"<<endl;
}

void ComputeCSFdensity::flipOuterSurface(int xFlip, int yFlip, int zFlip){
    cout<<"Flipping outer surface ..."<<endl;
    cout << "Input surface has " << m_outerSurface->GetNumberOfPoints() << " points." << endl;
    vtkSmartPointer<vtkPoints> points = vtkSmartPointer<vtkPoints>::New();

    for(vtkIdType j = 0; j < m_outerSurface->GetNumberOfPoints(); j++){
        double p[3];
        m_outerSurface->GetPoint(j,p);
        double P_M[3];
        P_M[0] = xFlip*p[0];    // x coordinate
        P_M[1] = yFlip*p[1];    // y coordinate
        P_M[2] = zFlip*p[2];    // z coordinate
        points->InsertNextPoint(P_M);
    }

    m_outerSurface->SetPoints(points);
    if (m_writeFlippedOuterSurface){
        string outputSurfaceFilename = m_output_dir + m_prefix + "_GM_Outer_MC_flipped.vtk";
        m_vio.writeFile(outputSurfaceFilename,m_outerSurface);
    }
    cout<<"Flipped outer surface created ..."<<endl;
}

void ComputeCSFdensity::computeStreamlines(int dims){
    SurfaceCorrespondence sCorr(m_whiteMatterSurface, m_outerSurface, dims, m_output_dir);
    //SurfaceCorrespondence sCorr(m_vio.readFile("WM.vtk"),m_vio.readFile("MC.vtk"), dims, "debug_test/");
    sCorr.setPrefix(m_prefix);
    sCorr.setPDEparams(0,10000,10000);
    sCorr.setWriteOptions(true);
    m_streamlines = sCorr.run();
}

void ComputeCSFdensity::readStreamLines(string streamFile){
    m_streamlines = m_vio.readFile(streamFile);
}

void ComputeCSFdensity::EstimateCortexStreamlinesDensity(int maxIter /* = 1*/, float maxDist /* = 20.0*/){
    // Start the clock
    int start_s=clock();

    // Input surface : white matter
    // inputPolyData <=> m_whiteMatterSurface
    cout << "Input surface has " << m_whiteMatterSurface->GetNumberOfPoints() << " points." << endl;

    // Streamlines
    cout << "Outer Streamlines File has " << m_streamlines ->GetNumberOfLines() << " lines." << endl;

    // Get the segmentation filename from the command line
    // inputImage <=> m_CSFprop

    // Get all segmentation data from the image

    typedef itk::Point< double, 3 > PointType;

    m_dImageType::Pointer inputImage = m_dImageType::New();
    inputImage->CopyInformation(m_CSFprop);
    inputImage->SetRegions(m_CSFprop->GetRequestedRegion());
    inputImage->Allocate();

    typedef itk::ImageRegionIterator< m_dImageType> IteratorType;
    IteratorType inputIt1(m_CSFprop, m_CSFprop->GetRequestedRegion());
    IteratorType inputIt2(inputImage, inputImage->GetRequestedRegion());

    inputIt1.GoToBegin();
    inputIt2.GoToBegin();
    while (!inputIt1.IsAtEnd())
    {
        inputIt2.Set(double(inputIt1.Get())/double(numeric_limits<unsigned short>::max()));
        ++inputIt1;
        ++inputIt2;
    }

    //cout << "Initializing Voxel Visiting Map... "  << endl;
    m_dImageType::Pointer outputImage = m_dImageType::New();
    outputImage->CopyInformation(inputImage);
    outputImage->SetRegions(inputImage->GetRequestedRegion());
    outputImage->Allocate();

    IteratorType outputIt(outputImage, outputImage->GetRequestedRegion());
    outputIt.GoToBegin();
    while (!outputIt.IsAtEnd())
    {
        outputIt.Set(0.0);
        ++outputIt;
    }

    // inputMask <=> m_ucseg
    m_dImageType::Pointer inputMask = m_dseg;

    //-----------------------------------------------Estimate CSF Density------------------------------------------------------------------

    vtkSmartPointer<vtkDoubleArray> array = vtkSmartPointer<vtkDoubleArray>::New();
    array->SetNumberOfComponents(1);
    array->SetName("CSFDensity");

    vtkSmartPointer<vtkCellArray> outerLineArray = m_streamlines->GetLines();
    vtkIdType outerLineID = -1;

    typedef itk::LinearInterpolateImageFunction<m_dImageType, double> InterpolatorType;

    for(vtkIdType vertexID = 0; vertexID < m_whiteMatterSurface->GetNumberOfPoints(); vertexID++){
        // cout << "Vertex ID " << vertexID << endl;
        double vertex_p[3];
        m_whiteMatterSurface->GetPoint(vertexID,vertex_p);
        outerLineID += 1;
        // cout << "Outer Line ID " << outerLineID << endl;

        vtkIdType lineOuterCellLocation = 0;
        outerLineArray->InitTraversal();
        int numOuter = 0;
        vtkIdType *lineIDlistOuterFinal = nullptr;

        for (vtkIdType a = 0; a <= outerLineID; a++){
            vtkIdType lineOuterNumIDs; // to hold the size of the cell
            vtkIdType *lineOuterIDlist; // to hold the ids in the cell
            outerLineArray->GetCell(lineOuterCellLocation, lineOuterNumIDs, lineOuterIDlist);
            lineOuterCellLocation += 1 + lineOuterNumIDs;
            if (a == outerLineID){
                // cout << "Line " << outerLineID << " has " << lineOuterNumIDs << " outer points." << endl;
                numOuter = lineOuterNumIDs;
                lineIDlistOuterFinal = lineOuterIDlist;
            }
        }

        InterpolatorType::Pointer Interpolator = InterpolatorType::New();
        Interpolator->SetInputImage(inputImage);

        int count = 0;
        int stopCount = numeric_limits<int>::max();
        double CSFDensity = 0.0;
        int outerFlag = 0;
        for(int a = 0; a < numOuter - 1; a++){
            int pointID = lineIDlistOuterFinal[a];
            double p[3];
            m_streamlines->GetPoint(pointID,p);
            int pointID_next = lineIDlistOuterFinal[a + 1];
            double p_next[3];
            m_streamlines->GetPoint(pointID_next,p_next);

            double step = vtkMath::Distance2BetweenPoints(p, p_next);

            if (a == 0){
                double squareMathDist = vtkMath::Distance2BetweenPoints(p, vertex_p);
                if (squareMathDist > 0.001){
                    outerFlag = 1;
                    break;
                    cout << "Not the correct Outer Line ID " << endl;
                }
            }

            PointType point;
            point[0] = -p[0];    // x coordinate
            point[1] = -p[1];    // y coordinate
            point[2] =  p[2];    // z coordinate

            PointType point_next;
            point_next[0] = -p[0];    // x coordinate
            point_next[1] = -p[1];    // y coordinate
            point_next[2] =  p[2];    // z coordinate

            m_dImageType::IndexType pixelIndex;

            m_dImageType::IndexType pixelIndex1;
            m_dImageType::PixelType label = inputMask->GetPixel(pixelIndex1);

            m_dImageType::PixelType probability = Interpolator->Evaluate(point);
            m_dImageType::PixelType probability_next = Interpolator->Evaluate(point_next);

            if(label > 0){
                CSFDensity += ((probability + probability_next)*step)/2;
                outputImage->SetPixel(pixelIndex, outerLineID); // Mark this pixel visited in current vertex
            }
            else{
                count+=1;
            }

            if (count > stopCount){
                break;
            }
        }

        if (CSFDensity < 0.001){
            CSFDensity = 0.0;
        }

        //cout << "CSFDensity = " << CSFDensity << endl;
        array->InsertNextValue(CSFDensity);

        if (outerFlag == 1){
            outerLineID-=1;
        }

    }

    m_whiteMatterSurface->GetPointData()->AddArray(array);

    //-----------------------------------------------Smoothe CSF Density-------------------------------------------------------------------

    for (int iter = 0; iter < maxIter; iter++){
        vtkSmartPointer<vtkDoubleArray> currentCSFDensity = vtkDoubleArray::SafeDownCast(m_whiteMatterSurface->GetPointData()->GetArray("CSFDensity"));;
        vtkSmartPointer<vtkDoubleArray> smoothedCSFDensity = vtkSmartPointer<vtkDoubleArray>::New();
        smoothedCSFDensity->SetNumberOfComponents(1);
        smoothedCSFDensity->SetName("CSFDensity");

        for(vtkIdType seed = 0; seed < m_whiteMatterSurface->GetNumberOfPoints(); seed++){
            vtkSmartPointer<vtkIdList> connectedVertices = vtkSmartPointer<vtkIdList>::New();

            //get all cells that vertex 'seed' is a part of
            vtkSmartPointer<vtkIdList> cellIdList = vtkSmartPointer<vtkIdList>::New();
            m_whiteMatterSurface->GetPointCells(seed, cellIdList);

            //loop through all the cells that use the seed point
            for(vtkIdType i = 0; i < cellIdList->GetNumberOfIds(); i++){
                vtkCell* cell = m_whiteMatterSurface->GetCell(cellIdList->GetId(i));

                //if the cell doesn't have any edges, it is a line
                if(cell->GetNumberOfEdges() <= 0){
                    continue;
                }

                for(vtkIdType e = 0; e < cell->GetNumberOfEdges(); e++){
                    vtkCell* edge = cell->GetEdge(e);

                    vtkIdList* pointIdList = edge->GetPointIds();

                    if(pointIdList->GetId(0) == seed || pointIdList->GetId(1) == seed)
                    {
                        if(pointIdList->GetId(0) == seed){
                            connectedVertices->InsertNextId(pointIdList->GetId(1));
                        }
                        else{
                            connectedVertices->InsertNextId(pointIdList->GetId(0));
                        }
                    }
                }
            }

            //cout << "There are " << connectedVertices->GetNumberOfIds() << " points connected to point " << seed << endl;
            double avgCSFDensity = currentCSFDensity->GetValue(seed);
            double seed_p[3];
            m_whiteMatterSurface->GetPoint(seed,seed_p);
            double fact = 1.0;
            for(vtkIdType ID = 0; ID < connectedVertices->GetNumberOfIds(); ID++){
                //cout << "Current Connected Vertex = " << connectedVertices->GetId(ID) << " to seed " << seed << endl;
                double seed_np[3];
                m_whiteMatterSurface->GetPoint(connectedVertices->GetId(ID),seed_np);
                double squaredDistance = vtkMath::Distance2BetweenPoints(seed_p, seed_np);
                double distance = sqrt(squaredDistance);
                double weight = (1-distance/maxDist) * int(distance<maxDist);

                avgCSFDensity += (currentCSFDensity->GetValue(connectedVertices->GetId(ID)) * weight);
                fact += weight;
            }
            avgCSFDensity/= fact;

            if (avgCSFDensity < 0.001){
                avgCSFDensity = 0.0;
            }

            smoothedCSFDensity->InsertNextValue(avgCSFDensity);
        }

        m_whiteMatterSurface->GetPointData()->AddArray(smoothedCSFDensity);
    }

    //-----------------------------------------------CSF Density Gradient------------------------------------------------------------------
    vtkSmartPointer<vtkAppendFilter> appendFilter = vtkSmartPointer<vtkAppendFilter>::New();
    appendFilter->AddInputData(m_whiteMatterSurface);
    appendFilter->Update();

    vtkSmartPointer<vtkUnstructuredGrid> unstructuredGrid = vtkSmartPointer<vtkUnstructuredGrid>::New();
    unstructuredGrid->ShallowCopy(appendFilter->GetOutput());

    vtkSmartPointer<vtkGradientFilter> gradients = vtkSmartPointer<vtkGradientFilter>::New();
    gradients->SetInputData(unstructuredGrid);
    gradients->SetInputScalars(0,"CSFDensity");
    gradients->SetResultArrayName("CSFDensityGradient");
    gradients->Update();

    vtkSmartPointer<vtkDoubleArray> arrayCSFDensity = vtkDoubleArray::SafeDownCast(m_whiteMatterSurface->GetPointData()->GetArray("CSFDensity"));

    vtkSmartPointer<vtkDoubleArray> arrayGradient = vtkDoubleArray::SafeDownCast(gradients->GetOutput()->GetPointData()->GetArray("CSFDensityGradient"));

    vtkSmartPointer<vtkDoubleArray> arrayMagGradient = vtkSmartPointer<vtkDoubleArray>::New();
    arrayMagGradient->SetNumberOfComponents(1);
    arrayMagGradient->SetName("CSFDensityMagGradient");

    vtkSmartPointer<vtkDoubleArray> arrayMagGradientNormalized = vtkSmartPointer<vtkDoubleArray>::New();
    arrayMagGradientNormalized->SetNumberOfComponents(1);
    arrayMagGradientNormalized->SetName("CSFDensityMagGradientNormalized");

    for(vtkIdType vertex = 0; vertex < m_whiteMatterSurface->GetNumberOfPoints(); vertex++){
        double g[3];
        g[0] = arrayGradient->GetComponent(vertex,0);
        g[1] = arrayGradient->GetComponent(vertex,1);
        g[2] = arrayGradient->GetComponent(vertex,2);
        double magGradient = vtkMath::Norm(g);
        double magGradientNormalized = magGradient/arrayCSFDensity->GetValue(vertex);

        if (magGradient == 0.0 || ::isnan(magGradient) || ::isnan(magGradientNormalized)){
            magGradient = 0;
            magGradientNormalized = 0;
        }

        arrayMagGradient->InsertNextValue(magGradient);
        arrayMagGradientNormalized->InsertNextValue(magGradientNormalized);

    }

    m_whiteMatterSurface->GetPointData()->AddArray(arrayMagGradient);
    m_whiteMatterSurface->GetPointData()->AddArray(arrayMagGradientNormalized);

    //-----------------------------------------------Output Results------------------------------------------------------------------
    string CSFDensityFilename = m_output_dir + m_prefix + "_CSFDensity.txt";
    ofstream CSFDensityFilestream;
    CSFDensityFilestream.open (CSFDensityFilename.c_str());
    CSFDensityFilestream << "NUMBER_OF_POINTS=" << m_whiteMatterSurface->GetNumberOfPoints() << endl;
    CSFDensityFilestream << "DIMENSION=1" << endl;
    CSFDensityFilestream << "TYPE=Scalar" << endl;
    for(vtkIdType vertex = 0; vertex < m_whiteMatterSurface->GetNumberOfPoints(); vertex++)
    {
        CSFDensityFilestream << arrayCSFDensity->GetValue(vertex) << endl;
    }
    CSFDensityFilestream.close();

    string CSFDMagGradientFilename = m_output_dir + m_prefix + "_CSFDensityMagGradient.txt";
    ofstream CSFDMagGradientFilestream;
    CSFDMagGradientFilestream.open (CSFDMagGradientFilename.c_str());
    CSFDMagGradientFilestream << "NUMBER_OF_POINTS=" << m_whiteMatterSurface->GetNumberOfPoints() << endl;
    CSFDMagGradientFilestream << "DIMENSION=1"  << endl;
    CSFDMagGradientFilestream << "TYPE=Scalar" << endl;
    for(vtkIdType vertex = 0; vertex < m_whiteMatterSurface->GetNumberOfPoints(); vertex++)
    {
        CSFDMagGradientFilestream << arrayMagGradient->GetValue(vertex) << endl;
    }
    CSFDMagGradientFilestream.close();

    std::string NormalizedMGFilename = m_output_dir + m_prefix + "_NormalizedCSFDensityMagGradient.txt";
    ofstream NormalizedMGFilestream;
    NormalizedMGFilestream.open (NormalizedMGFilename.c_str());
    NormalizedMGFilestream << "NUMBER_OF_POINTS=" << m_whiteMatterSurface->GetNumberOfPoints() << endl;
    NormalizedMGFilestream << "DIMENSION=1"  << endl;
    NormalizedMGFilestream << "TYPE=Scalar" << endl;
    for(vtkIdType vertex = 0; vertex < m_whiteMatterSurface->GetNumberOfPoints(); vertex++)
    {
        NormalizedMGFilestream << arrayMagGradientNormalized->GetValue(vertex) << endl;
    }
    NormalizedMGFilestream.close();

    if (m_writeOutputDensitySurf){
        string outputSurfaceFileName = m_output_dir + m_prefix + "_CSF_Density.vtk";
        m_vio.writeFile(outputSurfaceFileName, m_whiteMatterSurface);
    }

    if (m_writeVisitationMap){
        typedef itk::BinaryThresholdImageFilter <m_dImageType, m_dImageType> BinaryThresholdImageFilterType;

        BinaryThresholdImageFilterType::Pointer thresholdFilter = BinaryThresholdImageFilterType::New();
        thresholdFilter->SetInput(outputImage);
        thresholdFilter->SetLowerThreshold(1);
        thresholdFilter->SetUpperThreshold(numeric_limits<dPixelType>::max());
        thresholdFilter->SetInsideValue(1);
        thresholdFilter->SetOutsideValue(0);
        thresholdFilter->Update();

        string visitationMapFilename = m_output_dir + m_prefix + "_VisitationMap.nrrd";
        typedef itk::ImageFileWriter< m_dImageType > WriterType;
        WriterType::Pointer imageWriter = WriterType::New();
        imageWriter->SetInput(thresholdFilter->GetOutput());
        imageWriter->SetFileName(visitationMapFilename);
        imageWriter->Update();
    }

    int stop_s=clock();
    cout << "time: " << (((float)(stop_s-start_s))/CLOCKS_PER_SEC) <<" sec" << endl;
}


int main(int argc, char* argv[]) {
    if (argc != 7){
        cout << "Usage : " << argv[0] << " whiteMatterSurface greyMatterSurface segfile CSFprop prefix outputDir";
        return EXIT_FAILURE;
    }

    string WMsurf = argv[1];
    string segfile = argv[2];
    string csfProp = argv[3];
    string prefix = argv[4];
    string outputDir = argv[5];

    ComputeCSFdensity CSFdensity_LH(WMsurf, csfProp, segfile, prefix + "_LH", outputDir);
    CSFdensity_LH.createOuterImage(15,3);
    CSFdensity_LH.createOuterSurface(1);
    CSFdensity_LH.flipOuterSurface(-1,-1,1);
    CSFdensity_LH.computeStreamlines(300);
    CSFdensity_LH.readStreamLines(argv[7]);
    cout << "Starting cortex streamlines density estimation ..." << flush;
    CSFdensity_LH.EstimateCortexStreamlinesDensity(0,0);
    cout << " done" << endl;

    return EXIT_SUCCESS;
}
