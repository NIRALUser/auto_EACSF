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

ComputeCSFdensity::ComputeCSFdensity(string whiteMatterSurface_fileName, string greyMatterSurface_fileName, string segFile, string csfPropFile, string prefix, string output_dir){
    // Creating output directory if necessary
    int dirstatus = setOutputLocation(output_dir);
    if (dirstatus == EXIT_FAILURE){
        exit(1);
    }

    // Reading vtk surfaces
    m_whiteMatterSurface = m_vio.readFile(whiteMatterSurface_fileName);
    m_greyMatterSurface = m_vio.readFile(greyMatterSurface_fileName);


    // Reading itk images (segmentation  and probability map)
    typedef itk::ImageFileReader<m_ImageType> ReaderType;
    ReaderType::Pointer readerSeg = ReaderType::New();
    readerSeg->SetFileName(segFile);
    readerSeg->Update();
    m_seg = readerSeg->GetOutput();

    ReaderType::Pointer readerCSFprop = ReaderType::New();
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

void ComputeCSFdensity::shiftSurface(vtkPolyData *surf, double xShift, double yShift, double zShift, string outputFileName){
    cout << "Input surface has " << surf->GetNumberOfPoints() << " points." << endl;

    // Set up the transform filter
    vtkSmartPointer<vtkTransform> translation = vtkSmartPointer<vtkTransform>::New();
    translation->Translate(xShift, yShift, zShift);

    vtkSmartPointer<vtkTransformPolyDataFilter> transformFilter =
    vtkSmartPointer<vtkTransformPolyDataFilter>::New();
    transformFilter->SetInputData(surf);
    transformFilter->SetTransform(translation);
    transformFilter->Update();

    surf = transformFilter->GetOutput();

    if (outputFileName != ""){
        m_vio.writeFile(outputFileName,surf);
    }
}

void ComputeCSFdensity::translateSurfaces(double xShift, double yShift, double zShift){
    cout<<"Translating surfaces ..."<<endl;
    if (!m_writeTranslatedSurfaces){
        shiftSurface(m_whiteMatterSurface, xShift, yShift, zShift);
        shiftSurface(m_greyMatterSurface, xShift, yShift, zShift);
    }
    else{
        vector<string> filename = splitExt(m_WM_relFilename);
        shiftSurface(m_whiteMatterSurface, xShift, yShift, zShift, m_output_dir + filename[0] + "_translated" + filename[1]);
        filename = splitExt(m_GM_relFilename);
        shiftSurface(m_greyMatterSurface, xShift, yShift, zShift, m_output_dir + filename[0] + "_translated" + filename[1]);
    }
    cout<<"Translation done"<<endl;
}

void ComputeCSFdensity::createOuterImage(int closingradius, int dilationradius, bool reverse){
    cout<<"Creating outer image ..."<<endl;

    m_ImageType::SpacingType spacing = m_seg->GetSpacing();
    closingradius = closingradius/spacing[0];

    typedef itk::BinaryThresholdImageFilter <m_ImageType, m_ImageType> BinaryThresholdImageFilterType;
    BinaryThresholdImageFilterType::Pointer thresholdFilter = BinaryThresholdImageFilterType::New();
    thresholdFilter->SetInput(m_seg);
    thresholdFilter->SetLowerThreshold(1);
    thresholdFilter->SetUpperThreshold(4);
    thresholdFilter->SetInsideValue(1);
    thresholdFilter->SetOutsideValue(0);
    thresholdFilter->Update();

    m_outerImage = thresholdFilter->GetOutput();

    m_ImageType::SizeType regionSize;
    regionSize[0] = int(m_outerImage->GetLargestPossibleRegion().GetSize()[0]/2);
    regionSize[1] = m_outerImage->GetLargestPossibleRegion().GetSize()[1];
    regionSize[2] = m_outerImage->GetLargestPossibleRegion().GetSize()[2];

    m_ImageType::IndexType regionIndex;
    regionIndex[0] = 0;
    if (reverse){
        regionIndex[0] = m_outerImage->GetLargestPossibleRegion().GetSize()[0]/2;
    }
    regionIndex[1] = 0;
    regionIndex[2] = 0;

    m_ImageType::RegionType region;
    region.SetSize(regionSize);
    region.SetIndex(regionIndex);

    itk::ImageRegionIterator<m_ImageType> imageIterator(m_outerImage,region);

    while(!imageIterator.IsAtEnd()){
        imageIterator.Set(0);
        ++imageIterator;
    }

    typedef itk::BinaryBallStructuringElement<m_ImageType::PixelType,3> StructuringElementType;
    StructuringElementType structuringElement;
    structuringElement.SetRadius(closingradius);
    structuringElement.CreateStructuringElement();

    typedef itk::RescaleIntensityImageFilter<m_ImageType, m_ImageType > RescaleFilterType;
    RescaleFilterType::Pointer rescaleFilter = RescaleFilterType::New();
    rescaleFilter->SetInput(m_outerImage);
    rescaleFilter->SetOutputMinimum(0);
    rescaleFilter->SetOutputMaximum(255);
    rescaleFilter->Update();

    typedef itk::BinaryMorphologicalClosingImageFilter <m_ImageType, m_ImageType, StructuringElementType> BinaryMorphologicalClosingImageFilterType;
    BinaryMorphologicalClosingImageFilterType::Pointer closingFilter = BinaryMorphologicalClosingImageFilterType::New();
    closingFilter->SetInput(rescaleFilter->GetOutput());
    closingFilter->SetKernel(structuringElement);
    closingFilter->Update();

    dilationradius = dilationradius/spacing[0];

    structuringElement.SetRadius(dilationradius);
    structuringElement.CreateStructuringElement();

    typedef itk::BinaryDilateImageFilter <m_ImageType, m_ImageType, StructuringElementType> BinaryDilateImageFilterType;
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
        typedef itk::ImageFileWriter< m_ImageType >  WriterType;
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
    typedef itk::ImageToVTKImageFilter<m_ImageType> itkVtkConverter;
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
    sCorr.setPrefix(m_prefix);
    sCorr.setPDEparams(0,10000,10000);
    sCorr.setWriteOptions(true);
    m_streamlines = sCorr.run();
}

void ComputeCSFdensity::EstimateCortexStreamlinesDensity(int maxIter /* = 1*/, float maxDist /* = 20.0*/){
    // Start the clock
    int start_s=clock();

    // Input surface : white matter
    // inputPolyData <=> m_whiteMatterSurface
    cout << "Input surface has " << m_whiteMatterSurface->GetNumberOfPoints() << " points." << endl;

    // Streamlines
    cout << "Outer Streamlines File has " << m_streamlines ->GetNumberOfLines() << " lines." << endl;
    vtkDoubleArray* OuterLengthArray = vtkDoubleArray::SafeDownCast(m_streamlines->GetCellData()->GetArray("Length"));

    // Get the segmentation filename from the command line
    // inputImage <=> m_CSFprop

    // Get all segmentation data from the image
    const unsigned int Dimension = 3;
    typedef double dPixelType;
    typedef itk::Image< dPixelType, Dimension > dImageType;
    typedef itk::Point< double, Dimension > PointType;
    typedef itk::CastImageFilter<m_ImageType, dImageType> CastFilterType;

    CastFilterType::Pointer castFilter = CastFilterType::New();
    castFilter->SetInput(m_CSFprop);
    castFilter->Update();

    dImageType::Pointer inputImage = dImageType::New();
    inputImage->CopyInformation(castFilter->GetOutput());
    inputImage->SetRegions(castFilter->GetOutput()->GetRequestedRegion());
    inputImage->Allocate();

    typedef itk::ImageRegionIterator< dImageType> IteratorType;
    IteratorType inputIt1(castFilter->GetOutput(), castFilter->GetOutput()->GetRequestedRegion());
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
    dImageType::Pointer outputImage = dImageType::New();
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
    CastFilterType::Pointer maskCastFilter = CastFilterType::New();
    maskCastFilter->SetInput(m_seg);
    maskCastFilter->Update();
    dImageType::Pointer inputMask = maskCastFilter->GetOutput();

    //-----------------------------------------------Estimate CSF Density------------------------------------------------------------------

    vtkSmartPointer<vtkDoubleArray> array = vtkSmartPointer<vtkDoubleArray>::New();
    array->SetNumberOfComponents(1);
    array->SetName("CSFDensity");

    vtkSmartPointer<vtkCellArray> outerLineArray = m_streamlines->GetLines();
    vtkIdType outerLineID = -1;

    typedef itk::LinearInterpolateImageFunction<dImageType, double> InterpolatorType;

    for(vtkIdType vertexID = 0; vertexID < m_whiteMatterSurface->GetNumberOfPoints(); vertexID++){
        // cout << "Vertex ID " << vertexID << endl;
        double vertex_p[3];
        m_whiteMatterSurface->GetPoint(vertexID,vertex_p);
        outerLineID += 1;
        // cout << "Outer Line ID " << outerLineID << endl;

        vtkIdType lineOuterCellLocation = 0;
        outerLineArray->InitTraversal();
        int numOuter = 0;
        vtkIdType *lineIDlistOuterFinal;

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

            dImageType::IndexType pixelIndex;
            const bool isInside = inputImage->TransformPhysicalPointToIndex( point, pixelIndex );
            dImageType::PixelType visited = outputImage->GetPixel(pixelIndex);

            dImageType::IndexType pixelIndex1;
            const bool isInside1 = inputMask->TransformPhysicalPointToIndex( point, pixelIndex1 );
            dImageType::PixelType label = inputMask->GetPixel(pixelIndex1);

            dImageType::PixelType probability = Interpolator->Evaluate(point);
            dImageType::PixelType probability_next = Interpolator->Evaluate(point_next);

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
        typedef itk::BinaryThresholdImageFilter <dImageType, dImageType> BinaryThresholdImageFilterType;

        BinaryThresholdImageFilterType::Pointer thresholdFilter = BinaryThresholdImageFilterType::New();
        thresholdFilter->SetInput(outputImage);
        thresholdFilter->SetLowerThreshold(1);
        thresholdFilter->SetUpperThreshold(numeric_limits<dPixelType>::max());
        thresholdFilter->SetInsideValue(1);
        thresholdFilter->SetOutsideValue(0);
        thresholdFilter->Update();

        string visitationMapFilename = m_output_dir + m_prefix + "_VisitationMap.nrrd";
        typedef itk::ImageFileWriter< dImageType > WriterType;
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
    string GMsurf = argv[2];
    string segfile = argv[3];
    string csfProp = argv[4];
    string prefix = argv[5];
    string outputDir = argv[6];

    ComputeCSFdensity CSFdensity_LH(WMsurf, GMsurf, segfile, csfProp, prefix + "_LH", outputDir);
    // CSFdensity_LH.translateSurfaces(-194,-232,0);
    CSFdensity_LH.createOuterImage(15,8);
    CSFdensity_LH.createOuterSurface(1);
    CSFdensity_LH.flipOuterSurface(-1,-1,1);
    CSFdensity_LH.computeStreamlines(300);
    cout << "Starting cortex streamlines density estimation ..." << flush;
    CSFdensity_LH.EstimateCortexStreamlinesDensity(0,0);
    cout << " done" << endl;

    return EXIT_SUCCESS;
}
