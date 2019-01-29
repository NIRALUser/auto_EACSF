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
    ComputeCSFdensity(string whiteMatterSurface_fileName, string greyMatterSurface_fileName, string segFile, string prefix, string output_dir = "");
    void translateSurfaces(double xShift, double yShift, double zShift);
    void createOuterImage(int closingradius = 60, int dilationradius = 5, bool reverse = false);
    void createOuterSurface(int nbIterSmoothing);
    void flipOuterSurface(int xFlip, int yFlip, int zFlip);
    void computeStreamlines(int dims);

private:
    int setOutputLocation(string dirname);
    vector<string> splitExt(string filename);
    string relativePath(string path);
    void shiftSurface(vtkPolyData* surf, double xShift, double yShift, double zShift, string outputFileName = "");

    vtkIO m_vio;

    string m_WM_relFilename;
    string m_GM_relFilename;
    string m_prefix;
    string m_output_dir;

    vtkSmartPointer<vtkPolyData> m_whiteMatterSurface;
    vtkSmartPointer<vtkPolyData> m_greyMatterSurface;

    typedef itk::Image<unsigned char, 3> m_ImageType;
    m_ImageType::Pointer m_outerImage;

    vtkSmartPointer<vtkPolyData> m_outerSurface;

    bool m_writeTranslatedSurfaces = false;
    bool m_writeOuterImage = false;
    bool m_writeOuterSurface = false;
    bool m_writeFlippedOuterSurface = true;
};

#endif // COMPUTECSFDENSITY_H
