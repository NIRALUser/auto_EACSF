#include "geometry.h"

#include <vtkNew.h>
#include <vtkCell.h>
#include <vtkDataSet.h>
#include <vtkIdList.h>
#include <vtkType.h>

using namespace std;

size_t Geometry::extractNeighbors(vtkDataSet *ds, NeighborList &nbrs) {
	const size_t nPoints = ds->GetNumberOfPoints();
	nbrs.resize(nPoints);
	
	vtkNew<vtkIdList> cellIds;
	for (size_t j = 0; j < nPoints; j++) {
		Neighbors& nbrPts = nbrs[j];
		cellIds->Reset();
		ds->GetPointCells(j, cellIds.GetPointer());
		for (size_t k = 0; k < cellIds->GetNumberOfIds(); k++) {
			vtkIdType cellId = cellIds->GetId(k);
			vtkCell* cell = ds->GetCell(cellId);
			for (size_t l = 0; l < cell->GetNumberOfEdges(); l++) {
				vtkCell* edge = cell->GetEdge(l);
				vtkIdType s = edge->GetPointId(0);
				vtkIdType e = edge->GetPointId(1);
				if (s == j) {
					if (find(nbrPts.begin(), nbrPts.end(), e) == nbrPts.end()) {
						nbrPts.push_back(e);
					}
				} else if (e == j) {
					if (find(nbrPts.begin(), nbrPts.end(), s) == nbrPts.end()) {
						nbrPts.push_back(s);
					}
				}
			}
		}
	}
	
	return nPoints;
}