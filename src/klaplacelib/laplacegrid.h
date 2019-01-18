#ifndef LAPLACEGRID_H
#define LAPLACEGRID_H

#include <vector>

#include "geometry.h"

#include <vtkDataSet.h>
#include <vtkPolyData.h>

using namespace std;

class LaplaceGrid{
public:
    LaplaceGrid(double low, double high, double dt, vtkDataSet* dataSet, vtkPolyData* boundarySurface= NULL);

    void computeStep();

    void computeNormals(vtkDataSet* data);

    void computeExteriorNormals(vtkPolyData* boundarySurface, const double radius = .1);

    vtkDoubleArray* solution();

private:
    void initializeSolution();

    double m_low;
    double m_high;
    double m_dt;
    vtkDataSet* m_dataSet;
    vtkPolyData* m_samplePoints;

    vector<vtkIdType> m_solutionDomain;
    vtkIntArray* m_boundaryCond;
    vtkPolyData* m_boundarySurface;

    vtkDoubleArray* m_solution;
    vtkDoubleArray* m_tmpSolution;
    vtkDataArray* m_laplaceGradient;
    vtkDoubleArray* m_laplaceGradientNormals;


    Geometry m_geom;
    Geometry::NeighborList m_nbrs;
};

#endif // LAPLACEGRID_H
