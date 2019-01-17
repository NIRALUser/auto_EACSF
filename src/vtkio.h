//
//  vtkio.h
//  ktools
//
//  Created by Joohwi Lee on 12/5/13.
//
//

#ifndef __ktools__vtkio__
#define __ktools__vtkio__

#include <iostream>
//#include <vnl/vnl_matrix.h>

class vtkPolyData;
class vtkDataSet;
class vtkDataArray;
class vtkStructuredGrid;

class vtkIO {
public:
    /// @brief Rotate the object around z-axis. Change the sign of x and y coordinates. This works in place.
    //void zrotate(vtkPolyData* p);

    /// @brief Create a rotation matrix that rotates v1 to v2
    //void rotateVector(const double* v1, const double* v2, vnl_matrix<double>& rotation);


	vtkDataSet* readDataFile(std::string file);
    vtkPolyData* readFile(std::string file);
    //vtkDataArray* findFieldData(vtkPolyData* dataSet, std::string propertyName);
    void writeFile(std::string file, vtkDataSet* mesh);
    //void writeXMLFile(std::string file, vtkPolyData* mesh);
    
    //void writeXMLFile(std::string file, vtkStructuredGrid* grid);
//    vtkDataArray* getPointArray(vtkDataSet* ds);

};

#endif /* defined(__ktools__vtkio__) */
