#ifndef GRIDCREATE_H
#define GRIDCREATE_H

#include <vtkImageData.h>
#include <vtkPolyData.h>
#include <vtkStructuredGrid.h>

// compute the common voxel space
class GridCreate{
public:
    GridCreate(double* bounds, const int dims);

    void createImage(vtkImageData* im);

    vtkStructuredGrid* createStencil(vtkImageData* im, vtkPolyData* surf);

    int dim[3];
    double center[3];
    double spacing[3];
    int extent[6];
    double origin[3];
};

#endif
