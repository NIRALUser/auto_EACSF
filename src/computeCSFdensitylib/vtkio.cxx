//
//  vtkio.cpp
//  ktools
//
//  Created by Joohwi Lee on 12/5/13.
//
//

#include "vtkio.h"
#include <exception>
#include <stdexcept>

#include <vtkNew.h>
#include <vtkDataSet.h>
#include <vtkPoints.h>
#include <vtkPolyData.h>
#include <vtkImageData.h>
#include <vtkFieldData.h>
#include <vtkStructuredGrid.h>
#include <vtkUnstructuredGrid.h>
#include <vtkPolyDataReader.h>
#include <vtkPolyDataWriter.h>
#include <vtkXMLPolyDataReader.h>
#include <vtkXMLPolyDataWriter.h>
#include <vtkXMLUnstructuredGridReader.h>
#include <vtkXMLUnstructuredGridWriter.h>
#include <vtkXMLStructuredGridReader.h>
#include <vtkXMLStructuredGridWriter.h>
#include <vtkMNIObjectReader.h>
#include <vtkMNIObjectWriter.h>
#include <vtkMetaImageReader.h>
#include <vtkMetaImageWriter.h>
#include <vtkMath.h>
#include <vtkPointData.h>

static bool endswith(std::string file, std::string ext) {
   int epos = file.length() - ext.length();
    if (epos < 0) {
        return false;
    }
    return file.rfind(ext) == (unsigned int)epos;
}

vtkIO::vtkIO(){}

/// @brief Read a vtk/vtp file. The file's type is automatically determined by its extension.
vtkPolyData* vtkIO::readFile(std::string file) {
    if (endswith(file, ".vtp")) {
        vtkXMLPolyDataReader* r = vtkXMLPolyDataReader::New();
        r->SetFileName(file.c_str());
        r->Update();
        return r->GetOutput();
    } else if (endswith(file, ".vtk")) {
        vtkPolyDataReader* r = vtkPolyDataReader::New();
        r->SetFileName(file.c_str());
        r->Update();
        return r->GetOutput();
    } else if (endswith(file, ".obj")) {
        vtkMNIObjectReader* r = vtkMNIObjectReader::New();
        r->SetFileName(file.c_str());
        r->Update();
        return r->GetOutput();
	} else {
		throw std::runtime_error("file format not recognized by extesion");
	}
    return NULL;
}

/// @brief Write a vtk/vtp file. The file's type is automatically determined by its extension.
void vtkIO::writeFile(std::string file, vtkDataSet *mesh) {
    if (endswith(file, ".vtp")) {
        vtkXMLPolyDataWriter* w = vtkXMLPolyDataWriter::New();
        w->SetInputData(mesh);
        w->SetFileName(file.c_str());
        w->Write();
        w->Delete();
    } else if (endswith(file, ".vtk")) {
        vtkPolyDataWriter* w = vtkPolyDataWriter::New();
        w->SetInputData(mesh);
        w->SetFileName(file.c_str());
        w->Write();
        w->Delete();
    } else if (endswith(file, ".vtu")) {
        vtkXMLUnstructuredGridWriter* w = vtkXMLUnstructuredGridWriter::New();
        w->SetInputData(mesh);
        w->SetFileName(file.c_str());
        w->SetCompressorTypeToNone();
        w->SetDataModeToAscii();
        w->Write();
        w->Delete();
    } else if (endswith(file, ".vts")) {
        vtkXMLStructuredGridWriter* w = vtkXMLStructuredGridWriter::New();
        w->SetInputData(mesh);
        w->SetFileName(file.c_str());
		w->SetCompressorTypeToNone();
        w->Write();
        w->Delete();
	} else if (endswith(file, ".mhd")) {
		vtkMetaImageWriter* w = vtkMetaImageWriter::New();

		w->SetInputData(vtkImageData::SafeDownCast(mesh));
		w->SetFileName(file.c_str());
		w->Write();
		w->Delete();
	}
	cout << "Write " << file << " done ..." << endl;
}

vtkDataSet* vtkIO::readDataFile(std::string file) {
	if (endswith(file, ".vtp")) {
		vtkXMLPolyDataReader* r = vtkXMLPolyDataReader::New();
		r->SetFileName(file.c_str());
		r->Update();
		return r->GetOutput();
	} else if (endswith(file, ".vts")) {
		vtkXMLStructuredGridReader* r = vtkXMLStructuredGridReader::New();
		r->SetFileName(file.c_str());
		r->Update();
		return r->GetOutput();
	} else if (endswith(file, ".vtu")) {
		vtkXMLUnstructuredGridReader* r = vtkXMLUnstructuredGridReader::New();
		r->SetFileName(file.c_str());
		r->Update();
		return r->GetOutput();
	} else if (endswith(file, ".vtk")) {
		vtkPolyDataReader* r = vtkPolyDataReader::New();
		r->SetFileName(file.c_str());
		r->Update();
		return r->GetOutput();
	} else if (endswith(file, ".obj")) {
		vtkMNIObjectReader* r = vtkMNIObjectReader::New();
		r->SetFileName(file.c_str());
		r->Update();
		return r->GetOutput();
	} else if (endswith(file, ".mhd")) {
		vtkNew<vtkMetaImageReader> r;
		r->SetFileName(file.c_str());
		r->Update();
		vtkDataSet* ds = r->GetOutput();
		ds->Register(NULL);
		return ds;
	}
	cout << "Unknown file format: " << file << endl;
	return NULL;
}

