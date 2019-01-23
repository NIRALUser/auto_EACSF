#include "geometry.h"

#include <vtkNew.h>
#include <vtkCell.h>
#include <vtkDataSet.h>
#include <vtkIdList.h>
#include <vtkType.h>

using namespace std;

Geometry::Geometry() {}

size_t Geometry::extractNeighbors(vtkDataSet *ds, NeighborList &nbrs) {
    const vtkIdType nPoints = ds->GetNumberOfPoints();
	nbrs.resize(nPoints);
	
	vtkNew<vtkIdList> cellIds;
    for (vtkIdType j = 0; j < nPoints; j++) { // iterate through the points of the dataset
		Neighbors& nbrPts = nbrs[j];
		cellIds->Reset();
		ds->GetPointCells(j, cellIds.GetPointer());
        for (vtkIdType k = 0; k < cellIds->GetNumberOfIds(); k++) { // for each point, iterate through the cells using this point
			vtkIdType cellId = cellIds->GetId(k);
			vtkCell* cell = ds->GetCell(cellId);
            for (vtkIdType l = 0; l < cell->GetNumberOfEdges(); l++) { // for each cell, iterate through the edges of this cell
				vtkCell* edge = cell->GetEdge(l);
				vtkIdType s = edge->GetPointId(0);
				vtkIdType e = edge->GetPointId(1);

                // if one of the vertices in the edge is the starting point (j), add the other to its neighbor list if not there already
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
