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
    void surfacesTranslation(double xShift, double yShift, double zShift, bool writeOutputFiles);
    void createOuterImage(int closingradius = 60, int dilationradius = 5, bool reverse = false);

private:
    int setOuputLocation(string dirname);
    vector<string> splitExt(string filename);
    string relativePath(string path);
    void translateSurface(vtkPolyData* surf, double xShift, double yShift, double zShift, string outputFileName = "");

    vtkIO m_vio;

    string m_WM_relFilename;
    string m_GM_relFilename;
    string m_prefix;
    string m_output_dir;

    vtkPolyData* m_whiteMatterSurface = nullptr;
    vtkPolyData* m_greyMatterSurface = nullptr;

    typedef itk::Image<unsigned char, 3> m_ImageType;
    m_ImageType::Pointer m_outerImage;

    vtkPolyData* m_outerSurface;

    bool m_writeOuterImage = true;
};

#endif // COMPUTECSFDENSITY_H
