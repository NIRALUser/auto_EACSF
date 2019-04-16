#ifndef LAPLACEGRID_H
#define LAPLACEGRID_H

#include <vector>

#include "geometry.h"

#include <vtkDataSet.h>
#include <vtkPolyData.h>

using namespace std;

class LaplaceGrid{
public:
    /*** Sets the bounds of the temperature map (low, high) and the grid
     * Also, extract the neighborlist for each point of the data set */
    LaplaceGrid(double low, double high, vtkDataSet* dataSet);

    /*** For each point in the data set, sets its value to the average of the values its neighbors */
    void computeStep();

    /*** Compute the normals of the gradient of the solution */
    void computeNormals();

    /*** Returns the solution */
    vtkDoubleArray* solution();

private:
    /*** initialize the solution by setting the inner region to low, the outter region to high and the in between solution domain to 0 */
    void initializeSolution();

    double m_low;
    double m_high;
    vtkDataSet* m_dataSet;

    vector<vtkIdType> m_solutionDomain;
    vtkIntArray* m_boundaryCond;

    vtkDoubleArray* m_solution;
    vtkDoubleArray* m_tmpSolution;
    vtkDataArray* m_laplaceGradient;
    vtkDoubleArray* m_laplaceGradientNormals;

    Geometry::NeighborList m_nbrs;
};

#endif // LAPLACEGRID_H
