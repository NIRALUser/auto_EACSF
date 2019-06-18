#ifndef GRIDCREATE_H
#define GRIDCREATE_H

#include <vtkImageData.h>
#include <vtkPolyData.h>
#include <vtkStructuredGrid.h>

/*** compute the common voxel space */
class GridCreate{
public:
    /*** Sets the class attributes that describe a vtkStructured grid :
     *     - which size depends on the box defined by the array bounds
     *     - with dims number of cubes in the direction in which this box has the largest dimension
     *     - larger than the box by three cubes in each direction */
    GridCreate(double* bounds, const int dims);

    /*** Creates an 3D image defined by the class attributes :
     *     - number of pixel in each direction equal to the number cubes in that direction
     *     - spacing between pixels in each direction equal to the side of the cubes
     *     - origin of the image at the origin of the grid
     *     - only one color chanel, initialized at 255 for every pixels */
    void createImage(vtkImageData* im);

    /*** Creates a 3D binary image in which pixels are white at the locations of the input vtkPolyData surface and zero elswhere
     * then converts it to a vtkStructuredGrid and return the result */
    vtkStructuredGrid* createStencil(vtkPolyData* surf);

    double center[3]; // centers of the grid in each direction, defined as the middle of the bounds in each direction
    double spacing[3]; // side of the cubes thus constant in each direction, defined as the largest dimension of the box divided by dims
    int dim[3]; // number of cubes in each directions, defined as the dimension in each direction divided by the spacing
    int extent[6]; // normalized bounds of the grid, each smallest bound is 0 and the largest is the number of cubes in that absolute direction + 6 (+3 in each signed direction)
    double origin[3]; // actual starting position of the grid in each absolute direction
};

#endif
