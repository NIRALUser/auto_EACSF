#include "boundarycheck.h"

#include <vtkDataArray.h>
#include <vtkPointData.h>
#include <vtkPointLocator.h>

BoundaryCheck::BoundaryCheck(){

}

size_t BoundaryCheck::subtract(vtkDataSet *aim, vtkDataSet *bim){
    if (aim->GetNumberOfPoints() != bim->GetNumberOfPoints()) {
        cout << "can't process: the number of points are different!" << endl;
        return 0;
    }

    vtkDataArray* aarr = aim->GetPointData()->GetScalars();
    vtkDataArray* barr = bim->GetPointData()->GetScalars();

    size_t insideCount = 0;
    for (/*size_t*/int j = 0; j < aim->GetNumberOfPoints(); j++) {
        int p = aarr->GetTuple1(j);
        int q = barr->GetTuple1(j);
        int o = 700;
        if (p == 255 && q != 255) {
            o = 1;
            insideCount ++;
        } else if (p == 255 && q == 255){
            o = 300;
        }
        aarr->SetTuple1(j, o);
    }
    return insideCount;
}

void BoundaryCheck::checkSurface(vtkStructuredGrid *grid, vtkPolyData *isurf, vtkPolyData *osurf){
    vtkNew<vtkPointLocator> gridLoc;
    gridLoc->SetDataSet(grid);
    gridLoc->BuildLocator();

    vtkIntArray* sampledValue = vtkIntArray::SafeDownCast(grid->GetPointData()->GetScalars());

    const size_t nPoints = isurf->GetNumberOfPoints();
    size_t cnt = 0;

    for (size_t j = 0; j < nPoints; j++) {
        double p[3];

        isurf->GetPoint(j, p);
        vtkIdType pid = gridLoc->FindClosestPoint(p);

        int sample = sampledValue->GetValue(pid);
        if (sample == 300) {
            sampledValue->SetValue(pid, 1);
            cnt++;
        }
    }
    cout << "# of inside boundary correction: " << cnt << endl;

    cnt = 0;
    const size_t nPoints2 = osurf->GetNumberOfPoints();
    for (size_t j = 0; j < nPoints2; j++) {
        double p[3];

        osurf->GetPoint(j, p);
        vtkIdType pid = gridLoc->FindClosestPoint(p);

        int sample = sampledValue->GetValue(pid);
        if (sample == 700) {
            sampledValue->SetValue(pid, 1);
            cnt++;
        }
    }
    cout << "# of outside boundary correction: " << cnt << endl;
}
