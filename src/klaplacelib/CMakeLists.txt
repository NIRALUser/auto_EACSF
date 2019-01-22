list(APPEND klaplace_src
    klaplace.cxx
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

#add_library(klaplacelib ${klaplace_src})

find_package(Qt5 COMPONENTS Widgets OpenGL X11Extras REQUIRED)
set(QT_LIBRARIES ${Qt5Widgets_LIBRARIES})

FIND_PACKAGE(VTK REQUIRED)
INCLUDE(${VTK_USE_FILE})

add_executable(klaplace ${klaplace_src})

link_directories(${VTK_LIBRARY_DIRS})

#target_link_libraries(klaplace klaplacelib ${VTK_LIBRAIRES} ${QT_LIBRARIES})
target_link_libraries(klaplace ${VTK_LIBRARIES} ${QT_LIBRARIES})
