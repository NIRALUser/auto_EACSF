cmake_minimum_required (VERSION 2.8.11)
project (Auto_EACSF)

#Qt example------------------------------------

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY  ${CMAKE_CURRENT_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY  ${CMAKE_CURRENT_BINARY_DIR}/lib)

if(NOT INSTALL_RUNTIME_DESTINATION)
	set(INSTALL_RUNTIME_DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)
endif(NOT INSTALL_RUNTIME_DESTINATION)

if(NOT INSTALL_LIBRARY_DESTINATION)
	set(INSTALL_LIBRARY_DESTINATION ${CMAKE_INSTALL_PREFIX}/lib)
endif(NOT INSTALL_LIBRARY_DESTINATION)

if(NOT INSTALL_ARCHIVE_DESTINATION)
	set(INSTALL_ARCHIVE_DESTINATION ${CMAKE_INSTALL_PREFIX}/lib)
endif(NOT INSTALL_ARCHIVE_DESTINATION)

# Find the QtWidgets library

# find Qt5 headers
find_package(Qt5 COMPONENTS Widgets REQUIRED)

include_directories(${Qt5Widgets_INCLUDE_DIRS})
add_definitions(${Qt5Widgets_DEFINITIONS})

set(QT_LIBRARIES ${Qt5Widgets_LIBRARIES})

add_subdirectory(src)

set (version "1.0")

# Generate a bundle
if(CREATE_BUNDLE)

  if(APPLE)
    set(OS_BUNDLE MACOSX_BUNDLE)
  elseif(WIN32)
    set(OS_BUNDLE WIN32)
  endif()

  set(bundle_name Auto_EACSF${version})

  #--------------------------------------------------------------------------------
  SET(qtconf_dest_dir bin)
  SET(APPS "\${CMAKE_INSTALL_PREFIX}/bin/${bundle_name}")

  set(CPACK_PACKAGE_ICON "${myApp_ICON}")

  IF(APPLE)
    SET(qtconf_dest_dir ${bundle_name}.app/Contents/Resources)
    SET(APPS "\${CMAKE_INSTALL_PREFIX}/${bundle_name}.app")

    set(MACOSX_BUNDLE_BUNDLE_NAME "${bundle_name}")
    set(MACOSX_BUNDLE_INFO_STRING "Auto_EACSF: automatic extra axial cerebrospinal fluid segmentation.")
    set(MACOSX_BUNDLE_LONG_VERSION_STRING "Auto_EACSF version - ${version}")
    set(MACOSX_BUNDLE_SHORT_VERSION_STRING "${version}")
    set(MACOSX_BUNDLE_BUNDLE_VERSION "${version}")
    set(MACOSX_BUNDLE_COPYRIGHT "Copyright 2018 University of North Carolina , Chapel Hill.")
    
    set_source_files_properties(
      PROPERTIES
      MACOSX_PACKAGE_LOCATION Resources
      )
  ENDIF(APPLE)

  IF(WIN32)
    SET(APPS "\${CMAKE_INSTALL_PREFIX}/bin/${bundle_name}.exe")
  ENDIF(WIN32)

  include_directories(${CMAKE_CURRENT_BINARY_DIR})
  add_executable(${bundle_name} ${OS_BUNDLE} ${${APP_NAME}_src}
  )
  target_link_libraries(${bundle_name} ${QT_LIBRARIES} ${VTK_LIBRARIES})

  #--------------------------------------------------------------------------------
  # Install the QtTest application, on Apple, the bundle is at the root of the
  # install tree, and on other platforms it'll go into the bin directory.
  INSTALL(TARGETS ${bundle_name}
    DESTINATION . COMPONENT Runtime
    RUNTIME DESTINATION bin COMPONENT Runtime
  )

  macro(install_qt5_plugin _qt_plugin_name _qt_plugins_var)
    get_target_property(_qt_plugin_path "${_qt_plugin_name}" LOCATION)
    if(EXISTS "${_qt_plugin_path}")
      get_filename_component(_qt_plugin_file "${_qt_plugin_path}" NAME)
      get_filename_component(_qt_plugin_type "${_qt_plugin_path}" PATH)
      get_filename_component(_qt_plugin_type "${_qt_plugin_type}" NAME)
      set(_qt_plugin_dest "${bundle_name}.app/Contents/PlugIns/${_qt_plugin_type}")
      install(FILES "${_qt_plugin_path}"
        DESTINATION "${_qt_plugin_dest}"
        COMPONENT Runtime)
      set(${_qt_plugins_var}
        "${${_qt_plugins_var}};\${CMAKE_INSTALL_PREFIX}/${_qt_plugin_dest}/${_qt_plugin_file}")
    else()
      message(FATAL_ERROR "QT plugin ${_qt_plugin_name} not found")
    endif()
  endmacro()
  
  install_qt5_plugin("Qt5::QCocoaIntegrationPlugin" QT_PLUGINS)

  #--------------------------------------------------------------------------------
  # install a qt.conf file
  # this inserts some cmake code into the install script to write the file
  INSTALL(CODE "
      file(WRITE \"\${CMAKE_INSTALL_PREFIX}/${qtconf_dest_dir}/qt.conf\" \"[Paths]\nPlugins = PlugIns\n\")
      " COMPONENT Runtime)

  # Install the license
  INSTALL(FILES 
    ${CMAKE_SOURCE_DIR}/LICENSE
    DESTINATION "${CMAKE_INSTALL_PREFIX}/${qtconf_dest_dir}"
    COMPONENT Runtime)
 
  get_target_property(Qt5_location Qt5::Widgets LOCATION)
  string(FIND ${Qt5_location} "/QtWidgets" length)
  string(SUBSTRING ${Qt5_location} 0 ${length} Qt5_location)
  #Fix the bundle
  install(CODE "
    include(BundleUtilities)
    fixup_bundle(\"${APPS}\" \"${QT_PLUGINS};\" \"${Qt5_location}\")
   "
    COMPONENT Runtime)
 
  # To Create a package, one can run "cpack -G DragNDrop CPackConfig.cmake" on Mac OS X
  # where CPackConfig.cmake is created by including CPack
  # And then there's ways to customize this as well
  set(CPACK_BINARY_DRAGNDROP ON)

endif()


if(UNIX)
 # when building, don't use the install RPATH already
 # (but later on when installing)
 SET(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE) 
 # the RPATH to be used when installing
 SET(CMAKE_INSTALL_RPATH "../lib")
  
  get_target_property(QtWidgets_location Qt5::Widgets LOCATION)
  
  get_target_property(QtWidgets_SOFT Qt5::Widgets IMPORTED_SONAME_RELEASE)

  get_target_property(Qt5_location Qt5::Widgets LOCATION)  
  string(FIND ${Qt5_location} "libQt5Widgets" length)
  string(SUBSTRING ${Qt5_location} 0 ${length} Qt5_location)
  
  #install(FILES ${QtWidgets_location} ${QtOpenGL_location} ${Qt5_location}/${QtWidgets_SOFT} ${Qt5_location}/${QtOpenGL_SOFT}
  #    DESTINATION lib
  #    COMPONENT Runtime)

  install(FILES ${QtWidgets_location} ${QtOpenGL_location}
      DESTINATION lib
      COMPONENT Runtime)

endif()

# To Create a package, one can run "cpack -G DragNDrop CPackConfig.cmake" on Mac OS X
# where CPackConfig.cmake is created by including CPack
# And then there's ways to customize this as well  
include(InstallRequiredSystemLibraries)
include(CPack) 


configure_file(CMake/Auto_EACSFConfig.cmake.in
  "${PROJECT_BINARY_DIR}/Auto_EACSFConfig.cmake" @ONLY)

if(WIN32 AND NOT CYGWIN)
  set(INSTALL_CMAKE_DIR cmake)
else()
  set(INSTALL_CMAKE_DIR lib/cmake/Auto_EACSF)
endif()

install(FILES
  "${PROJECT_BINARY_DIR}/Auto_EACSFConfig.cmake"  
  DESTINATION "${INSTALL_CMAKE_DIR}" COMPONENT dev)


file(GLOB Auto_EACSF_HEADERS "*.h")
install(FILES ${Auto_EACSF_HEADERS} 
DESTINATION include)
