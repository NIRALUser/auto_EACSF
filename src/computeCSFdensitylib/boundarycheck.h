#ifndef BOUNDARYCHECK_H
#define BOUNDARYCHECK_H

#include <vtkDataSet.h>
#include <vtkPolyData.h>
#include <vtkStructuredGrid.h>

/* Creates a region map with three regions : inner (WM), outter, and in-between (GM) */
class BoundaryCheck
{
public:
    BoundaryCheck();

    /*** Based on the inputs, two binary grids that represent the white and the grey matters, assigns labels to the different regions :
     *     - 700 for outter region (for which each corresponding voxel had value 0 in both inputs)
     *     - 1 for grey matter region (for which each corresponding voxel had value 255 only in the grey matter input)
     *     - 300 for inner, white matter region (for which each voxel had value 255 in both inputs)
     * Instead of creating a new vtkDataSet for the output, the first input is modified and holds the result of the labelling.
     * The output of the function is the number voxels in the in-between region. */
    size_t subtract(vtkDataSet* gim, vtkDataSet* wim);

    /*** Correct the potential boundary errors by iterating through :
     *     - the elements of the inner surface, get the closest point of the grid to that element, and labelling it 1 if not already at 1
     *     - the elements of the outter surface, get the closest point of the grid to that element, and labelling it 1 if not already at 1 */
    void checkSurface(vtkStructuredGrid* grid, vtkPolyData* isurf,  vtkPolyData* osurf);
};

#endif // BOUNDARYCHECK_H
