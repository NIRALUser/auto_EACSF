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

find_package(SlicerExecutionModel REQUIRED)
include(${SlicerExecutionModel_USE_FILE})

SEMMacroBuildCLI(
  NAME computecsfdensity
  EXECUTABLE_ONLY
  TARGET_LIBRARIES ${ITK_LIBRARIES} ${VTK_LIBRARIES} ${QT_LIBRARIES}
  ADDITIONAL_SRCS ${computeCSFdensity_src}
  RUNTIME_OUTPUT_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
  LIBRARY_OUTPUT_DIRECTORY ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}
  ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_ARCHIVE_OUTPUT_DIRECTORY}
  INSTALL_RUNTIME_DESTINATION ${INSTALL_RUNTIME_DESTINATION}
  INSTALL_LIBRARY_DESTINATION ${INSTALL_LIBRARY_DESTINATION}
  INSTALL_ARCHIVE_DESTINATION ${INSTALL_ARCHIVE_DESTINATION}
)

#add_executable(computeCSFdensity ${computeCSFdensity_src})

#link_directories(${VTK_LIBRARY_DIRS} ${ITK_LIBRARIY_DIRS})

#target_link_libraries(computeCSFdensity ${ITK_LIBRARIES} ${VTK_LIBRARIES} ${QT_LIBRARIES})
