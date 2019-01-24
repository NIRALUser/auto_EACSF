#ifndef __SURFACECORRESPONDENCE_H__

#include <set>

#include <vtkDataSet.h>
#include <vtkModifiedBSPTree.h>
#include <vtkPolyData.h>

#include "vtkio.h"

using namespace std;

class SurfaceCorrespondance{
    typedef std::vector<std::string> StringVector;

public:
    SurfaceCorrespondance(string inputObj1, string inputObj2, int dims = 300);

    void setPrefix(string prefix);
    void setWriteOptions(bool writeGridFile, bool writeLaplaceFieldFile, bool writeStreamFile, bool writeWarpedMeshFile, bool writeObjFile);

private:
    /*** create a structured grid with the size of input
     * convert the grid to polydata
     * create the intersection between the grid and the polydata
     * outputs the resulting grid */
    void createGrid();

    /*** computes the Laplace field from the previous grid */
    void computeLaplacePDE(vtkDataSet* data, const double low, const double high, const int nIters);
    bool performLineClipping(vtkPolyData* streamLines, vtkModifiedBSPTree* tree, /*int lineId,*/ vtkCell* lineToClip, vtkPoints* outputPoints, vtkCellArray* outputLines, double &length);
    vtkPolyData* performStreamTracerPostProcessing(vtkPolyData* streamLines, vtkPolyData* seedPoints, vtkPolyData* destinationSurface);

    /*** computes the stream lines */
    void performStreamTracer();
    void findNeighborPoints(vtkCell* cell, vtkIdType pid, set<vtkIdType>& nbrs);
    void interpolateBrokenPoints(vtkPolyData* surf, vtkPoints* warpedPoints, vtkDataArray* seedIds);

    vtkIO m_vio;

    string m_prefix = "surface_correspondence";
    int m_dims; //number of subdivision of the grid in each dimensions

    vtkPolyData* m_isurf;
    vtkPolyData* m_osurf;

    vtkDataSet* m_laplaceField;
    vtkPolyData* m_streams;

    bool m_writeGridFile = false;
    bool m_writeLaplaceFieldFile = false;
    bool m_writeStreamFile = false;
    bool m_writeWarpedMeshFile = false;
    bool m_writeObjFile = false;

public:
    void run();
};

#endif
