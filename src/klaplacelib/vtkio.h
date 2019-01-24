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
    vtkIO();
	vtkDataSet* readDataFile(std::string file);
    vtkPolyData* readFile(std::string file);
    void writeFile(std::string file, vtkDataSet* mesh);

};

#endif /* defined(__ktools__vtkio__) */
