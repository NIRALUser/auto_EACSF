#include "laplacegrid.h"

#include <vtkCellData.h>
#include <vtkCellLocator.h>
#include <vtkFloatArray.h>
#include <vtkGradientFilter.h>
#include <vtkPointData.h>
#include <vtkPointLocator.h>
#include <vtkPolyDataNormals.h>
#include <vtkThresholdPoints.h>

LaplaceGrid::LaplaceGrid(double low, double high, vtkDataSet *dataSet):
    m_low(low),
    m_high(high),
    m_dataSet(dataSet)
{
    cout << "geometry edge extraction ... " << flush;
    Geometry::extractNeighbors(dataSet, m_nbrs);
    cout << " done " << endl;

    // check boundary points
    m_boundaryCond = vtkIntArray::SafeDownCast(dataSet->GetPointData()->GetArray("SampledValue"));
    if (m_boundaryCond == NULL) {
        throw runtime_error("No scalar values for BoundaryPoints");
    }

    initializeSolution();
}

void LaplaceGrid::initializeSolution(){
    cout << "initializing solution grid ... " << flush;
    // low-value 2
    // high-value 1
    m_solution = vtkDoubleArray::New();
    m_solution->SetName("LaplacianSolution");
    m_solution->SetNumberOfComponents(1);
    m_solution->SetNumberOfTuples(m_boundaryCond->GetNumberOfTuples());
    m_solution->FillComponent(0, 0);

    m_tmpSolution = vtkDoubleArray::New();
    m_tmpSolution->SetName("LaplacianSolution");
    m_tmpSolution->SetNumberOfComponents(1);
    m_tmpSolution->SetNumberOfTuples(m_boundaryCond->GetNumberOfTuples());
    m_tmpSolution->FillComponent(0, 0);

    const size_t nPts = m_boundaryCond->GetNumberOfTuples();
    for (size_t j = 0; j < nPts; j++) {
        int domain = m_boundaryCond->GetValue(j);
        double uValue = 0;
        if (domain == 700) {
            // high
            uValue = m_high;
        } else if (domain == 300){
            // low
            uValue = m_low;
        } else if (domain == 1) {
            uValue = 0;
            m_solutionDomain.push_back(j);
        }
        m_solution->SetValue(j, uValue);
        m_tmpSolution->SetValue(j, uValue);
    }
    cout << "# of points: " << m_solutionDomain.size() << endl;
}

void LaplaceGrid::computeStep(){
    const size_t nPts = m_solutionDomain.size();
    for (size_t j = 0; j < nPts; j++) {
        vtkIdType centerId = m_solutionDomain[j];
        Geometry::Neighbors& edgeMap = m_nbrs[centerId];
        Geometry::Neighbors::iterator iter = edgeMap.begin();

        double u = 0;
        double nNbrs = 0;
        for (; iter != edgeMap.end(); iter++) {
            const double du = m_solution->GetValue(*iter);
            u += du;
            nNbrs ++;
        }
        u = u / nNbrs;
        m_tmpSolution->SetValue(centerId, u);
    }

    vtkDoubleArray* swapTmp = m_tmpSolution;
    m_tmpSolution = m_solution;
    m_solution = swapTmp;
}

void LaplaceGrid::computeNormals(){
    cout<<"Computing normals ..."<<flush;
    vtkNew<vtkGradientFilter> gradFilter;
    gradFilter->SetInputData(m_dataSet);
    gradFilter->SetInputScalars(vtkDataSet::FIELD_ASSOCIATION_POINTS, "LaplacianSolution");
    gradFilter->SetResultArrayName("LaplacianGradient");
    gradFilter->Update();
    m_laplaceGradient = gradFilter->GetOutput()->GetPointData()->GetArray("LaplacianGradient");
    m_laplaceGradient->Register(NULL);

    m_laplaceGradientNormals = vtkDoubleArray::New();
    m_laplaceGradientNormals->SetName("LaplacianGradientNorm");
    m_laplaceGradientNormals->SetNumberOfComponents(3);
    m_laplaceGradientNormals->SetNumberOfTuples(m_laplaceGradient->GetNumberOfTuples());

    const size_t nPts = m_laplaceGradientNormals->GetNumberOfTuples();
    for (size_t j = 0; j < nPts; j++) {
        double* vec = m_laplaceGradient->GetTuple3(j);
        double norm = vtkMath::Norm(vec);
        if (norm > 1e-10) {
            m_laplaceGradientNormals->SetTuple3(j, vec[0]/norm, vec[1]/norm, vec[2]/norm);
        } else {
            m_laplaceGradientNormals->SetTuple3(j, 0, 0, 0);

        }
    }

    m_dataSet->GetPointData()->AddArray(m_laplaceGradient);
    m_dataSet->GetPointData()->SetVectors(m_laplaceGradientNormals);
    cout<<" done"<<endl;
}

vtkDoubleArray* LaplaceGrid::solution(){
    return m_solution;
}
