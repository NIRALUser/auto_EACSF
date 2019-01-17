#include <map>

#include <vtkDataSet.h>
#include <vtkDoubleArray.h>
#include <vtkType.h>
#include <vtkNew.h>
#include <vtkPolyData.h>
#include <vtkTransform.h>

class Geometry {
public:
    vtkNew<vtkTransform> txfm;
	
	struct Edge {
		vtkIdType u;
		vtkIdType v;
		bool boundary;
        size_t axisAligned;
		
		bool operator==(const Edge& e) const {
			return (u == e.u && v == e.v) || (u == e.v && v == e.u);
		}
		
		bool operator<(const Edge& e) const {
			return (u == e.u ? v < e.v : u < e.u);
		}
		
		bool operator<=(const Edge& e) const {
			return (u == e.u ? v <= e.v : u <= e.u);
		}
		
		Edge(): u(-1), v(-1), boundary(false) {}
		Edge(vtkIdType s, vtkIdType e): u(s), v(e), boundary(false), axisAligned(0) {}
		Edge(vtkIdType s, vtkIdType e, bool b): u(s), v(e), boundary(b), axisAligned(0) {}
		Edge(const Edge& e): u(e.u), v(e.v), boundary(e.boundary), axisAligned(0) {}
		
		void operator=(const Edge& e) {
			u = e.u;
			v = e.v;
            boundary = e.boundary;
            axisAligned = e.axisAligned;
		}
	};
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