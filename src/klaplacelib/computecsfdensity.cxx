#include "computecsfdensity.h"

#include <iostream>

#include "surfacecorrespondence.h"

using namespace std;

//void EstimateCortexStreamlinesDensity(vtkPolyData* inputPolyData, vtkPolyData* outerstreamlinesPolyData, string InputSegmentationFileName, string InputMaskFileName, string OutputSurfacename, string OutputVoxelVistitingMap){

//}

computeCSFdensity::computeCSFdensity()
{

}

int main(int argc, char* argv[]) {
    //cout<<"inputs: "<<endl<<inputObj1<<endl<<inputObj2<<endl<<prefix<<endl<<dims<<endl<<endl;

//    if (argc != 9)
//    {
//        cout << "Usage : " << argv[0];
//    }

    string inputObj1 = argv[1];
    string inputObj2 = argv[2];
    string prefix = argv[3];
    int dims = atoi(argv[4]);
//    string segFile = argv[5];
//    string maskFile = argv[6];
//    string outSurface = argv[7];
//    string outVisitingMap = argv[8];

    SurfaceCorrespondance sCorr(inputObj1,inputObj2,dims);
    if (prefix != "")
    {
        sCorr.setPrefix(prefix);
    }
    sCorr.setPDEparams(0,10000,10000);
    sCorr.run();

    vtkPolyData* streamlines = sCorr.streams();
    vtkPolyData* surface = sCorr.isurf();
//    cout << "Starting cortex streamlines density estimation ..." << flush;
//    EstimateCortexStreamlinesDensity(surface,streamlines, segFile, maskFile, outSurface, outVisitingMap);
//    cout << " done" << endl;
}
