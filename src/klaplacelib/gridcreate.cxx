#include "gridcreate.h"

#include <vtkImageStencil.h>
#include <vtkImageToStructuredGrid.h>
#include <vtkMetaImageWriter.h>
#include <vtkNew.h>
#include <vtkPolyDataToImageStencil.h>
#include <vtkPointData.h>

using namespace std;

GridCreate::GridCreate(double* bounds, const int dims){
    double maxbound = max(bounds[1]-bounds[0], max(bounds[3]-bounds[2], bounds[5]-bounds[4]));

    center[0] = (bounds[1]+bounds[0])/2.0;
    center[1] = (bounds[3]+bounds[2])/2.0;
    center[2] = (bounds[5]+bounds[4])/2.0;

    double gridSpacing = maxbound / dims;
    spacing[0] = spacing[1] = spacing[2] = gridSpacing;

    dim[0] = (bounds[1]-bounds[0])/gridSpacing;
    dim[1] = (bounds[3]-bounds[2])/gridSpacing;
    dim[2] = (bounds[5]-bounds[4])/gridSpacing;

    extent[0] = extent[2] = extent[4] = 0;
    extent[1] = dim[0] + 6;
    extent[3] = dim[1] + 6;
    extent[5] = dim[2] + 6;

    origin[0] = bounds[0] + gridSpacing*-3;
    origin[1] = bounds[2] + gridSpacing*-3;
    origin[2] = bounds[4] + gridSpacing*-3;

    cout << "Grid Dimension: " << dims << "; Grid Spacing: " << gridSpacing << " dim: " << dim[0] << "," << dim[1] << "," << dim[2] <<endl;
}

void GridCreate::createImage(vtkImageData* im) {
    im->SetSpacing(spacing);
    im->SetExtent(extent);
    im->SetOrigin(origin);
    im->AllocateScalars(VTK_INT, 1);
    im->GetPointData()->GetScalars()->FillComponent(0, 255);
}

vtkStructuredGrid* GridCreate::createStencil(vtkImageData* im, vtkPolyData* surf) {
    createImage(im);

    vtkNew<vtkPolyDataToImageStencil> psten;
    psten->SetInputData(surf);
    psten->SetOutputOrigin(origin);
    psten->SetOutputSpacing(spacing);
    psten->SetOutputWholeExtent(extent);
    psten->Update();

    vtkNew<vtkImageStencil> isten;
    isten->SetInputData(im);
    isten->SetStencilData(psten->GetOutput());
    isten->ReverseStencilOff();
    isten->SetBackgroundValue(0);
    isten->Update();


    vtkImageData* imgGrid = isten->GetOutput();
    // visualiseDataSet(imgGrid);
    vtkNew<vtkMetaImageWriter> imageWriter;
    imageWriter->SetInputData(imgGrid);
    imageWriter->SetFileName("testimage.mhd");
    imageWriter->Write();
    imgGrid->GetPointData()->GetScalars()->SetName("SampledValue");


    vtkNew<vtkImageToStructuredGrid> imgToGrid;
    imgToGrid->SetInputData(imgGrid);
    imgToGrid->Update();

    vtkStructuredGrid* output = imgToGrid->GetOutput();
    output->GetPointData()->SetScalars(imgGrid->GetPointData()->GetScalars());
    output->Register(NULL);

    return output;
}
