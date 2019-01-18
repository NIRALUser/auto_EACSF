#include "laplacegrid.h"

#include <vtkCellData.h>
#include <vtkCellLocator.h>
#include <vtkFloatArray.h>
#include <vtkGradientFilter.h>
#include <vtkPointData.h>
#include <vtkPointLocator.h>
#include <vtkPolyDataNormals.h>
#include <vtkThresholdPoints.h>

LaplaceGrid::LaplaceGrid(double low, double high, double dt, vtkDataSet *dataSet, vtkPolyData *boundarySurface):
    m_low(low),
    m_high(high),
    m_dt(dt),
    m_dataSet(dataSet),
    m_boundarySurface(boundarySurface)
{
    cout << "geometry edge extraction ... " << flush;
    m_geom.extractNeighbors(dataSet, m_nbrs);
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

            //                    cout << iter->second.axisAligned << endl;
        }
        u = u / nNbrs;
        m_tmpSolution->SetValue(centerId, u);
    }

    vtkDoubleArray* swapTmp = m_tmpSolution;
    m_tmpSolution = m_solution;
    m_solution = swapTmp;
//			memcpy(m_solution->WritePointer(0, nPts), m_tmpSolution->GetVoidPointer(0), sizeof(double) * nTuples);
//			m_solution->DeepCopy(m_tmpSolution);
}

void LaplaceGrid::computeNormals(vtkDataSet *data){
    /*
     vtkNew<vtkCellDerivatives> deriv;
     deriv->SetInput(data);
     deriv->SetVectorModeToComputeGradient();
     deriv->Update();
     vtkDataSet* derivOut = deriv->GetOutput();
     derivOut->GetCellData()->SetActiveVectors("ScalarGradient");
     vtkDataArray* scalarGradient = deriv->GetOutput()->GetCellData()->GetArray("ScalarGradient");
     scalarGradient->SetName("LaplacianGradient");
     */

    vtkNew<vtkGradientFilter> gradFilter;
    gradFilter->SetInputData(data);
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

    data->GetPointData()->AddArray(m_laplaceGradient);
    data->GetPointData()->SetVectors(m_laplaceGradientNormals);
}

void LaplaceGrid::computeExteriorNormals(vtkPolyData *boundarySurface, const double radius){
    if (boundarySurface == NULL) {
        return;
    }
    vtkNew<vtkPolyDataNormals> normalsFilter;
    normalsFilter->SetInputData(boundarySurface);
    normalsFilter->ComputeCellNormalsOn();
    normalsFilter->ComputePointNormalsOn();
    normalsFilter->Update();
    vtkFloatArray* cellNormals = vtkFloatArray::SafeDownCast(normalsFilter->GetOutput()->GetCellData()->GetNormals());

    vtkNew<vtkCellLocator> cloc;
    cloc->SetDataSet(boundarySurface);
    cloc->AutomaticOn();
    cloc->BuildLocator();

    m_dataSet->GetPointData()->SetActiveScalars("SampledValue");

    vtkNew<vtkThresholdPoints> threshold;
    threshold->SetInputData(m_dataSet);
    threshold->ThresholdByUpper(250);
    threshold->Update();
    vtkDataSet* inoutBoundary = threshold->GetOutput();
    vtkIntArray* inoutBoundaryCond = vtkIntArray::SafeDownCast(inoutBoundary->GetPointData()->GetArray("SampledValue"));


    vtkNew<vtkPointLocator> ploc;
    ploc->SetDataSet(inoutBoundary);
    ploc->AutomaticOn();
    ploc->BuildLocator();

    const size_t nPts = m_dataSet->GetNumberOfPoints();
    for (size_t j = 0; j < nPts; j++) {
        int domain = m_boundaryCond->GetValue(j);
        if (domain == 700 || domain == 300 || domain == 0) {
            double x[3] = { 0, }, closestPoint[3] = { 0, };
            vtkIdType cellId = -1;
            int subId = 0;
            double dist2 = -1;
            m_dataSet->GetPoint(j, x);
            //vtkNew<vtkGenericCell> closestCell;
            cloc->FindClosestPointWithinRadius(x, radius, closestPoint, cellId, subId, dist2);

            float cellNormal[3];
            cellNormals->GetTypedTuple(cellId, cellNormal);
            cellNormal[0] = 0;
            vtkMath::Normalize(cellNormal);

            if (domain == 0) {
                vtkIdType xId = ploc->FindClosestPoint(x);
                domain = inoutBoundaryCond->GetValue(xId);
                assert(domain == 300 || domain == 700);
            }


            if (domain == 300) {
                m_laplaceGradientNormals->SetTuple3(j, -cellNormal[0], -cellNormal[1], -cellNormal[2]);
            } else {
                m_laplaceGradientNormals->SetTuple3(j, cellNormal[0], cellNormal[1], cellNormal[2]);
            }
        }
    }
}

vtkDoubleArray* LaplaceGrid::solution(){
    return m_solution;
}
