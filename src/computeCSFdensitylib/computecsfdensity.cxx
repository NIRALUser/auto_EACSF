#include "computecsfdensity.h"
#include "surfacecorrespondence.h"

#include <iostream>
#include <sys/types.h>
#include <sys/stat.h>

#include "vtkDecimatePro.h"
#include "vtkMarchingCubes.h"
#include "vtkSmoothPolyDataFilter.h"
#include "vtkTransform.h"
#include "vtkTransformPolyDataFilter.h"
#include "vtkPolyDataWriter.h"

#include "itkBinaryBallStructuringElement.h"
#include "itkBinaryMorphologicalClosingImageFilter.h"
#include "itkBinaryThresholdImageFilter.h"
#include "itkImageFileReader.h"
#include "itkImageFileWriter.h"
#include "itkImageRegionIterator.h"
#include "itkImageToVTKImageFilter.h"
#include "itkRescaleIntensityImageFilter.h"

#include "surfacecorrespondence.h"

using namespace std;

//void EstimateCortexStreamlinesDensity(vtkPolyData* inputPolyData, vtkPolyData* outerstreamlinesPolyData, string InputSegmentationFileName, string InputMaskFileName, string OutputSurfacename, string OutputVoxelVistitingMap){

//}

ComputeCSFdensity::ComputeCSFdensity(string whiteMatterSurface_fileName, string greyMatterSurface_fileName, string segFile, string prefix, string output_dir){
    // Creating output directory if necessary
    int dirstatus = setOuputLocation(output_dir);
    if (dirstatus == EXIT_FAILURE){
        exit(1);
    }

    // Reading vtk surfaces
    m_whiteMatterSurface = m_vio.readFile(whiteMatterSurface_fileName);
    m_greyMatterSurface = m_vio.readFile(greyMatterSurface_fileName);


    // Reading itk image
    typedef itk::ImageFileReader<m_ImageType> ReaderType;
    ReaderType::Pointer reader = ReaderType::New();
    reader->SetFileName(segFile);
    reader->Update();
    m_outerImage = reader->GetOutput();

    // Other members setting
    m_WM_relFilename = relativePath(whiteMatterSurface_fileName);
    m_GM_relFilename = relativePath(greyMatterSurface_fileName);

    m_prefix = prefix;
}

int ComputeCSFdensity::setOuputLocation(string dirname){
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

    m_ImageType::SpacingType spacing = m_outerImage->GetSpacing();
    closingradius = closingradius/spacing[0];

    typedef itk::BinaryThresholdImageFilter <m_ImageType, m_ImageType> BinaryThresholdImageFilterType;
    BinaryThresholdImageFilterType::Pointer thresholdFilter = BinaryThresholdImageFilterType::New();
    thresholdFilter->SetInput(m_outerImage);
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

    if (m_writeOuterImage){
        string hemisphere = "_LH";
        if (reverse){
            hemisphere = "_RH";
        }
        string filename = m_output_dir + m_prefix + hemisphere + "_GM_Dilated.nrrd";
        typedef itk::ImageFileWriter< m_ImageType >  WriterType;
        WriterType::Pointer writer = WriterType::New();
        writer->SetFileName(filename);
        writer->SetInput( rescaleFilter->GetOutput() );
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
        string outputSurfaceFilename = m_output_dir + m_prefix + "_LH_GM_Outer_MC.vtk";
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
        string outputSurfaceFilename = m_output_dir + m_prefix + "_LH_GM_Outer_MC_flipped.vtk";
        m_vio.writeFile(outputSurfaceFilename,m_outerSurface);
    }
    cout<<"Flipped outer surface created ..."<<endl;
}

void ComputeCSFdensity::computeStreamlines(int dims){
    SurfaceCorrespondance sCorr(m_whiteMatterSurface, m_outerSurface, dims);
    sCorr.setPrefix(m_prefix);
    sCorr.setPDEparams(0,10000,10000);
    sCorr.setWriteOptions(true);
    sCorr.run();
}

int main(int argc, char* argv[]) {
    if (argc != 6){
        cout << "Usage : " << argv[0] << " whiteMatterSurface greyMatterSurface segfile prefix outputDir";
        return EXIT_FAILURE;
    }

    string WMsurf = argv[1];
    string GMsurf = argv[2];
    string segfile = argv[3];
    string prefix = argv[4];
    string outputDir = argv[5];

    ComputeCSFdensity CSFdensity_LH(WMsurf, GMsurf, segfile, prefix, outputDir);
    CSFdensity_LH.translateSurfaces(-194,-232,0);
    CSFdensity_LH.createOuterImage(15,8);
    CSFdensity_LH.createOuterSurface(1);
    CSFdensity_LH.flipOuterSurface(-1,-1,1);
    CSFdensity_LH.computeStreamlines(300);
    //cout<<"inputs: "<<endl<<inputObj1<<endl<<inputObj2<<endl<<prefix<<endl<<dims<<endl<<endl;

//    if (argc != 9)
//    {
//        cout << "Usage : " << argv[0];
//    }


//    string prefix = argv[3];
//    int dims = atoi(argv[4]);
//    string segFile = argv[5];
//    string maskFile = argv[6];
//    string outSurface = argv[7];
//    string outVisitingMap = argv[8];

//    SurfaceCorrespondance sCorr(inputObj1,inputObj2,dims);
//    if (prefix != "")
//    {
//        sCorr.setPrefix(prefix);
//    }
//    sCorr.setPDEparams(0,10000,10000);
//    sCorr.run();

//    vtkPolyData* streamlines = sCorr.streams();
//    vtkPolyData* surface = sCorr.whiteMatterSurface();
//    cout << "Starting cortex streamlines density estimation ..." << flush;
//    EstimateCortexStreamlinesDensity(surface,streamlines, segFile, maskFile, outSurface, outVisitingMap);
//    cout << " done" << endl;
    return EXIT_SUCCESS;
}
