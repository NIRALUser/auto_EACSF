#ifndef BOUNDARYCHECK_H
#define BOUNDARYCHECK_H

#include <vtkDataSet.h>
#include <vtkPolyData.h>
#include <vtkStructuredGrid.h>

class BoundaryCheck
{
public:
    BoundaryCheck();

    size_t subtract(vtkDataSet* aim, vtkDataSet* bim);

    void checkSurface(vtkStructuredGrid* grid, vtkPolyData* isurf,  vtkPolyData* osurf);
};

#endif // BOUNDARYCHECK_H
