#include "computecsfdensity.h"

#include <iostream>
#include <sys/types.h>
#include <sys/stat.h>

#include "vtkTransform.h"
#include "vtkTransformPolyDataFilter.h"

#include "itkBinaryBallStructuringElement.h"
#include "itkBinaryMorphologicalClosingImageFilter.h"
#include "itkBinaryThresholdImageFilter.h"
#include "itkImageFileReader.h"
#include "itkImageRegionIterator.h"
#include "itkImageFileWriter.h"
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

void ComputeCSFdensity::translateSurface(vtkPolyData *surf, double xShift, double yShift, double zShift, string outputFileName){
    std::cout << "Input surface has " << surf->GetNumberOfPoints() << " points." << std::endl;

    // Set up the transform filter
    vtkSmartPointer<vtkTransform> translation = vtkSmartPointer<vtkTransform>::New();
    translation->Translate(xShift, yShift, zShift);

    vtkSmartPointer<vtkTransformPolyDataFilter> transformFilter =
    vtkSmartPointer<vtkTransformPolyDataFilter>::New();
    transformFilter->SetInputData(surf);
    transformFilter->SetTransform(translation);
    transformFilter->Update();

    if (outputFileName != ""){
        m_vio.writeFile(outputFileName,transformFilter->GetOutput());
    }
}

void ComputeCSFdensity::surfacesTranslation(double xShift, double yShift, double zShift, bool writeOutputFiles){
    cout<<"Translating surfaces ..."<<endl;
    if (!writeOutputFiles){
        translateSurface(m_whiteMatterSurface, xShift, yShift, zShift);
        translateSurface(m_greyMatterSurface, xShift, yShift, zShift);
    }
    else{
        vector<string> filename = splitExt(m_WM_relFilename);
        translateSurface(m_whiteMatterSurface, xShift, yShift, zShift, m_output_dir + filename[0] + "_translated" + filename[1]);
        filename = splitExt(m_GM_relFilename);
        translateSurface(m_greyMatterSurface, xShift, yShift, zShift, m_output_dir + filename[0] + "_translated" + filename[1]);
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
        typedef itk::ImageFileWriter< m_ImageType >  WriterType;
        WriterType::Pointer writer = WriterType::New();
        writer->SetFileName(m_output_dir + m_prefix + hemisphere + "_GM_Dilated.nrrd");
        writer->SetInput( rescaleFilter->GetOutput() );
        writer->UseCompressionOn ();
        try
          {
          writer->Update();
          }
        catch( itk::ExceptionObject & e )
          {
          std::cerr << "Error: " << e << std::endl;
          }
    }

    cout<<"Outer image created"<<endl;
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

    ComputeCSFdensity CSFdensity(WMsurf, GMsurf, segfile, prefix, outputDir);
    CSFdensity.surfacesTranslation(-194,-232,0,false);
    CSFdensity.createOuterImage(15,8);

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
