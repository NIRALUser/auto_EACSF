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
    vtkNew<vtkTransform> txfm;

	typedef std::map<vtkIdType, Edge> EdgeMap;
    typedef std::vector<EdgeMap> EdgeList;
	typedef std::vector<vtkIdType> Neighbors;
	typedef std::vector<std::vector<vtkIdType> > NeighborList;
	
    // size_t extractEdges(vtkDataSet* ds, EdgeList& edges);
	size_t extractNeighbors(vtkDataSet* ds, NeighborList& nbrs);
    
    /*
    double tangentVector(const double u[3], const double v[3], const double n[3], double tv[3], vtkTransform* txf = NULL);
	
	double rotateVector(const double p[3], const double q[3], vtkTransform* txf, double* crossOut = NULL);
	
	double rotatePlane(const double u1[3], const double v1[3], const double u2[3], const double u3[3], vtkTransform* txf);
    
    double normalizeToNorthPole(const double u[3], const double n[3], double* cross, vtkTransform* txf);
    
    double denormalizeFromNorthPole(const double u[3], const double n[3], double* cross, vtkTransform* txf);
	
	
	bool computeContourNormals(vtkDataSet* ds, vtkDoubleArray* normals);
	
	vtkPolyData* computeSurfaceNormals(vtkPolyData* pd);
	
	void print();
	*/
};
