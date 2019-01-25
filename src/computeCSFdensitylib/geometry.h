#include <map>

#include <vtkDataSet.h>
#include <vtkDoubleArray.h>
#include <vtkType.h>
#include <vtkNew.h>
#include <vtkPolyData.h>
#include <vtkTransform.h>

#include "edge.h"

class Geometry {
public:
    typedef std::map<vtkIdType, Edge> EdgeMap;
    typedef std::vector<EdgeMap> EdgeList;
    typedef std::vector<vtkIdType> Neighbors;
    typedef std::vector<std::vector<vtkIdType> > NeighborList;

    Geometry();

    static size_t extractNeighbors(vtkDataSet* ds, NeighborList& nbrs);

private:
    vtkNew<vtkTransform> txfm;
};
