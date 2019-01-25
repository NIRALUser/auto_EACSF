list(APPEND computeCSFdensity_src
    computecsfdensity.cxx
    computecsfdensity.h
    surfacecorrespondence.cxx
    surfacecorrespondence.h
    geometry.cxx
    geometry.h
    edge.cxx
    edge.h
    vtkio.cxx
    vtkio.h
    gridcreate.cxx
    gridcreate.h
    boundarycheck.cxx
    boundarycheck.h
    laplacegrid.cxx
    laplacegrid.h
)

#add_library(computeCSFdensitylib ${computeCSFdensity_src})

find_package(Qt5 COMPONENTS Widgets OpenGL X11Extras REQUIRED)
set(QT_LIBRARIES ${Qt5Widgets_LIBRARIES})

find_package(VTK REQUIRED)
include(${VTK_USE_FILE})

find_package(ITK REQUIRED)
include(${ITK_USE_FILE})

add_executable(computeCSFdensity ${computeCSFdensity_src})

link_directories(${VTK_LIBRARY_DIRS} ${ITK_LIBRARIY_DIRS})

#target_link_libraries(computeCSFdensity klaplacelib ${VTK_LIBRAIRES} ${QT_LIBRARIES})
target_link_libraries(computeCSFdensity ${ITK_LIBRARIES} ${VTK_LIBRARIES} ${QT_LIBRARIES})
