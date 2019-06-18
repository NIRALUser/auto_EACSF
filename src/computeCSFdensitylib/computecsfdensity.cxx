#include "computecsfdensity.h"
#include "computecsfdensityCLP.h"
#include "surfacecorrespondence.h"

#include <cstdlib>
#include <fstream>

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
#include "vtkPolyDataReader.h"
#include "vtkGenericDataObjectReader.h"

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
    ucReaderType::Pointer readerSeg = ucReaderType::New();
    readerSeg->SetFileName(segFile);
    readerSeg->Update();
    m_seg = readerSeg->GetOutput();

    typedef itk::ImageFileReader<m_dImageType> dReaderType;
    dReaderType::Pointer readerCSFprop = dReaderType::New();
    readerCSFprop->SetFileName(csfPropFile);
    readerCSFprop->Update();
    m_CSFprop = readerCSFprop->GetOutput();

    // Other members setting
    m_WM_relFilename = relativePath(whiteMatterSurface_fileName);

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
    string filename = m_output_dir + m_prefix + "_GM_Dilated.nrrd";
    if (SurfaceCorrespondence::fileExists(filename))
    {
        cout<<"Outer image already exists"<<endl;
        cout<<"Loading "<<filename<<" ..."<<endl;
        typedef itk::ImageFileReader<m_ucImageType> ucReaderType;
        ucReaderType::Pointer outerImageReader = ucReaderType::New();
        outerImageReader->SetFileName(filename);
        outerImageReader->Update();
        m_outerImage = outerImageReader->GetOutput();
        cout<<"Outer image loaded"<<endl;
    }
    else{
        cout<<"Creating outer image ..."<<endl;

        m_ucImageType::SpacingType spacing = m_seg->GetSpacing();
        closingradius = closingradius/spacing[0];

        typedef itk::BinaryThresholdImageFilter <m_ucImageType, m_ucImageType> BinaryThresholdImageFilterType;
        BinaryThresholdImageFilterType::Pointer thresholdFilter = BinaryThresholdImageFilterType::New();
        thresholdFilter->SetInput(m_seg);
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
}

void ComputeCSFdensity::createOuterSurface(int nbIterSmoothing){
    string outputSurfaceFilename = m_output_dir + m_prefix + "_GM_Outer_MC.vtk";
    if (SurfaceCorrespondence::fileExists(outputSurfaceFilename)){
        cout<<"Outer surface already exists"<<endl;
        cout<<"Loading "<<outputSurfaceFilename<<" ..."<<endl;
        m_outerSurface = m_vio.readFile(outputSurfaceFilename);
        cout<<"Outer surface loaded"<<endl;
    }
    else{
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
            m_vio.writeFile(outputSurfaceFilename,m_outerSurface);
        }

        cout<<"Outer surface created"<<endl;
    }
}

void ComputeCSFdensity::flipOuterSurface(int xFlip, int yFlip, int zFlip){
    string outputSurfaceFilename = m_output_dir + m_prefix + "_GM_Outer_MC_flipped.vtk";
    if (SurfaceCorrespondence::fileExists(outputSurfaceFilename)){
        cout<<"Flipped outer surface already exists"<<endl;
        cout<<"Loading "<<outputSurfaceFilename<<" ..."<<endl;
        m_outerSurface = m_vio.readFile(outputSurfaceFilename);
        cout<<"Flipped outer surface loaded"<<endl;
    }
    else{
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
            m_vio.writeFile(outputSurfaceFilename,m_outerSurface);
        }
        cout<<"Flipped outer surface created ..."<<endl;
    }
}

void ComputeCSFdensity::computeStreamlines(int dims){
    string streamsFilename = m_output_dir + m_prefix + "_stream.vtk";
    if (SurfaceCorrespondence::fileExists(streamsFilename))
    {
        cout<<"Streamlines already exists"<<endl;
        cout<<"Loading "<<streamsFilename<<" ..."<<endl;
        m_streamlines = m_vio.readFile(streamsFilename);
        cout<<"Streamlines loaded"<<endl;
    }
    else{
        cout<<"Computing streamlines"<<endl;
        SurfaceCorrespondence sCorr(m_whiteMatterSurface, m_outerSurface, dims, m_output_dir);
        //SurfaceCorrespondence sCorr(m_vio.readFile("WM.vtk"),m_vio.readFile("MC.vtk"), dims, "debug_test/");
        sCorr.setOutputStreamFilename(streamsFilename);
        sCorr.setPrefix(m_prefix);
        sCorr.setPDEparams(0,10000,10000);
        sCorr.setWriteOptions(true);
        m_streamlines = sCorr.run();
        cout<<"Streamlines computed"<<endl;
    }
}

void ComputeCSFdensity::readStreamLines(string streamFile){
    m_streamlines = m_vio.readFile(streamFile);
}

void ComputeCSFdensity::EstimateCortexStreamlinesDensity(int maxIter /* = 1*/, float maxDist /* = 20.0*/){

    string CSFDensityFilename = m_output_dir + m_prefix + "_CSFDensity.txt";
    string CSFDMagGradientFilename = m_output_dir + m_prefix + "_CSFDensityMagGradient.txt";
    string NormalizedMGFilename = m_output_dir + m_prefix + "_NormalizedCSFDensityMagGradient.txt";
    string outputSurfaceFileName = m_output_dir + m_prefix + "_CSF_Density.vtk";
    string visitationMapFilename = m_output_dir + m_prefix + "_VisitationMap.nrrd";

    if ((!SurfaceCorrespondence::fileExists(CSFDensityFilename) && m_writeDensity)
            || (!SurfaceCorrespondence::fileExists(CSFDMagGradientFilename) && m_writeDensityMagGradient)
            || (!SurfaceCorrespondence::fileExists(NormalizedMGFilename) && m_writeNormDensityMagGradient)
            || (!SurfaceCorrespondence::fileExists(outputSurfaceFileName) && m_writeOutputDensitySurf)
            || (!SurfaceCorrespondence::fileExists(visitationMapFilename) && m_writeVisitationMap))
    {
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
        inputImage = m_CSFprop;

        typedef itk::ImageRegionIterator< m_dImageType> IteratorType;
        IteratorType inputIt(inputImage, inputImage->GetRequestedRegion());
        inputIt.GoToBegin();
        while (!inputIt.IsAtEnd())
        {
            inputIt.Set(double(inputIt.Get())/double(numeric_limits<unsigned short>::max()));
            ++inputIt;
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

        // inputMask <=> m_seg

        itk::CastImageFilter<m_ucImageType, m_dImageType>::Pointer castFilter = itk::CastImageFilter<m_ucImageType, m_dImageType>::New();
        castFilter->SetInput(m_seg);
        castFilter->Update();
        m_dImageType::Pointer inputMask = castFilter->GetOutput();

        //m_dImageType::Pointer inputMask = m_dseg;

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
                inputImage->TransformPhysicalPointToIndex( point, pixelIndex );
                outputImage->GetPixel(pixelIndex);

                m_dImageType::IndexType pixelIndex1;
                inputMask->TransformPhysicalPointToIndex( point, pixelIndex1 );
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

        if (m_writeDensity){
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
        }

        if (m_writeDensityMagGradient){
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
        }

        if (m_writeNormDensityMagGradient){
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
        }

        if (m_writeOutputDensitySurf){
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

            typedef itk::ImageFileWriter< m_dImageType > WriterType;
            WriterType::Pointer imageWriter = WriterType::New();
            imageWriter->SetInput(thresholdFilter->GetOutput());
            imageWriter->SetFileName(visitationMapFilename);
            imageWriter->Update();
        }

        int stop_s=clock();
        cout << "time: " << (((float)(stop_s-start_s))/CLOCKS_PER_SEC) <<" sec" << endl;
    }
    else{
        cout << "Already existing cortex streamlines density estimation detected." << endl;
    }

}

void ComputeCSFdensity::setWriteOptions(bool writeOuterImage, bool writeOuterSurface, bool writeFlippedOuterSurface, bool writeDensity, bool writeDensityMagGradient,
                                   bool writeNormDensityMagGradient, bool writeOutputDensitySurf, bool writeVisitationMap)
{
    m_writeOuterImage = writeOuterImage;
    m_writeOuterSurface = writeOuterSurface;
    m_writeFlippedOuterSurface = writeFlippedOuterSurface;
    m_writeDensity = writeDensity;
    m_writeDensityMagGradient = writeDensityMagGradient;
    m_writeNormDensityMagGradient = writeNormDensityMagGradient;
    m_writeOutputDensitySurf = writeOutputDensitySurf;
    m_writeVisitationMap = writeVisitationMap;
}

void ComputeCSFdensity::setWriteOptions(bool writeAll)
{
    m_writeOuterImage = writeAll;
    m_writeOuterSurface = writeAll;
    m_writeFlippedOuterSurface = writeAll;
    m_writeDensity = writeAll;
    m_writeDensityMagGradient = writeAll;
    m_writeNormDensityMagGradient = writeAll;
    m_writeOutputDensitySurf = writeAll;
    m_writeVisitationMap = writeAll;
}

int main(int argc, char* argv[]) {
    PARSE_ARGS;
    if (CSFprobabilityMap.empty()){
        cerr << "A CSF probability map has to be specified" << endl;
        return EXIT_FAILURE;
    }

    if (segmentation.empty()){
        cerr << "A brain segmentation has to be specified" << endl;
        return EXIT_FAILURE;
    }

    if (LH_WM_Surf.empty() && RH_WM_Surf.empty()){
        cerr << "At least one inner surface has to be specified" << endl;
        return EXIT_FAILURE;
    }

    if (prefix.empty()){
        prefix = "CCDresult";
    }

    if (!LH_WM_Surf.empty()){
        cout << "Computing CSF density for left hemisphere ..." <<endl;
        ComputeCSFdensity CSFdensity_LH(LH_WM_Surf, segmentation, CSFprobabilityMap, prefix + "_LH", outputDir);
        if (LH_streamlines.empty()){
            cout << "Computing left hemisphere streamlines ..." << endl;
            CSFdensity_LH.createOuterImage(15,3);
            CSFdensity_LH.createOuterSurface(1);
            CSFdensity_LH.flipOuterSurface(-1,-1,1);
            CSFdensity_LH.computeStreamlines(300);
        }
        else{
            cout << "Reading left hemisphere streamlines ..." << endl;
            CSFdensity_LH.readStreamLines(LH_streamlines);
        }
        cout << "Starting cortex streamlines density estimation ..." << endl;
        CSFdensity_LH.EstimateCortexStreamlinesDensity(0,0);
        cout << "Left hemisphere done" << endl;
    }

    if (!RH_WM_Surf.empty()){
        cout << "Computing CSF density for right hemisphere ..." <<endl;
        ComputeCSFdensity CSFdensity_RH(RH_WM_Surf, segmentation, CSFprobabilityMap, prefix + "_RH", outputDir);
        if (RH_streamlines.empty()){
            cout << "Computing right hemisphere streamlines ..." << endl;
            CSFdensity_RH.createOuterImage(1,3,true);
            CSFdensity_RH.createOuterSurface(1);
            CSFdensity_RH.flipOuterSurface(-1,-1,1);
            CSFdensity_RH.computeStreamlines(300);
        }
        else{
            cout << "Reading right hemisphere streamlines ..." << endl;
            CSFdensity_RH.readStreamLines(RH_streamlines);
        }
        cout << "Starting cortex streamlines density estimation ..." << endl;
        CSFdensity_RH.EstimateCortexStreamlinesDensity(0,0);
        cout << "Right hemisphere done" << endl;
    }

    return EXIT_SUCCESS;
}
