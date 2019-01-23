#ifndef __SURFACECORRESPONDENCE_H__

#include <set>

#include <vtkDataSet.h>
#include <vtkModifiedBSPTree.h>
#include <vtkPolyData.h>

using namespace std;

class SurfaceCorrespondance{
    typedef std::vector<std::string> StringVector;

public:
    SurfaceCorrespondance(string inputObj1, string inputObj2, string prefix = "surface_correspondence", int dims = 300);

private:
    vtkDataSet* createGrid(vtkPolyData* osurf, vtkPolyData* isurf, const int dims, size_t& insideCountOut);

    /*** create a structured grid with the size of input
     * convert the grid to polydata
     * create the intersection between the grid and the polydata
     * outputs the resulting grid */
    vtkDataSet *runFillGrid(StringVector& args, int dims, bool writeGridFile);



    void computeLaplacePDE(vtkDataSet* data, const double low, const double high, const int nIters);
    bool performLineClipping(vtkPolyData* streamLines, vtkModifiedBSPTree* tree, /*int lineId,*/ vtkCell* lineToClip, vtkPoints* outputPoints, vtkCellArray* outputLines, double &length);
    vtkPolyData* performStreamTracerPostProcessing(vtkPolyData* streamLines, vtkPolyData* seedPoints, vtkPolyData* destinationSurface);
    vtkPolyData* performStreamTracer(vtkDataSet* inputData, vtkPolyData* inputSeedPoints, vtkPolyData* destSurf, bool zRotate = false);
    void findNeighborPoints(vtkCell* cell, vtkIdType pid, set<vtkIdType>& nbrs);
    void interpolateBrokenPoints(vtkPolyData* surf, vtkPoints* warpedPoints, vtkDataArray* seedIds);
    void runPrintTraceCorrespondence(string inputMeshName, string inputStreamName, string outputWarpedMeshName, vtkPolyData* srcmesh = NULL);

    string m_inputObj1; //white matter surface
    string m_inputObj2; //grey matter surface
    string m_prefix;
    int m_dims; //number of subdivision of the grid in each dimensions

public:
    void run();
};

#endif
