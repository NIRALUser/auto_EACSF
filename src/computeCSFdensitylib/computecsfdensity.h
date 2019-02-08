#ifndef COMPUTECSFDENSITY_H
#define COMPUTECSFDENSITY_H

#include <string>
#include <vector>

#include "vtkPolyData.h"

#include "itkImage.h"

#include "vtkio.h"

using namespace std;

class ComputeCSFdensity
{
public:
    ComputeCSFdensity(string whiteMatterSurface_fileName, string greyMatterSurface_fileName, string segFile, string csfPropFile, string prefix, string output_dir = "");
    void createOuterImage(int closingradius = 60, int dilationradius = 5, bool reverse = false);
    void createOuterSurface(int nbIterSmoothing);
    void flipOuterSurface(int xFlip, int yFlip, int zFlip);
    void computeStreamlines(int dims);
    void EstimateCortexStreamlinesDensity(int maxIter = 1, float maxDist = 20.0);
    void readStreamLines(string streamFile);

private:
    int setOutputLocation(string dirname);
    vector<string> splitExt(string filename);
    string relativePath(string path);

    vtkIO m_vio;

    string m_WM_relFilename;
    string m_GM_relFilename;
    string m_prefix;
    string m_output_dir;

    vtkSmartPointer<vtkPolyData> m_whiteMatterSurface;
    vtkSmartPointer<vtkPolyData> m_greyMatterSurface;

    typedef itk::Image< unsigned char, 3 > m_ucImageType;
    typedef double dPixelType;
    typedef itk::Image< dPixelType, 3 > m_dImageType;
    m_ucImageType::Pointer m_ucseg;
    m_dImageType::Pointer m_dseg;
    m_dImageType::Pointer m_CSFprop;
    m_ucImageType::Pointer m_outerImage;

    vtkSmartPointer<vtkPolyData> m_outerSurface;

    vtkSmartPointer<vtkPolyData> m_streamlines;

    bool m_writeOuterImage = true;
    bool m_writeOuterSurface = true;
    bool m_writeFlippedOuterSurface = true;
    bool m_writeOutputDensitySurf = true;
    bool m_writeVisitationMap = true;
};

#endif // COMPUTECSFDENSITY_H
