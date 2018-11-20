cmake_minimum_required (VERSION 2.8.11)
project (Auto_EACSF)

#Qt example------------------------------------

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY  ${CMAKE_CURRENT_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY  ${CMAKE_CURRENT_BINARY_DIR}/lib)


if(NOT INSTALL_RUNTIME_DESTINATION)
	set(INSTALL_RUNTIME_DESTINATION bin)
endif(NOT INSTALL_RUNTIME_DESTINATION)

if(NOT INSTALL_LIBRARY_DESTINATION)
	set(INSTALL_LIBRARY_DESTINATION lib)
endif(NOT INSTALL_LIBRARY_DESTINATION)

if(NOT INSTALL_ARCHIVE_DESTINATION)
	set(INSTALL_ARCHIVE_DESTINATION lib)
endif(NOT INSTALL_ARCHIVE_DESTINATION)


# Find the QtWidgets library

# find Qt5 headers
find_package(Qt5 COMPONENTS Widgets REQUIRED)

include_directories(${Qt5Widgets_INCLUDE_DIRS})
add_definitions(${Qt5Widgets_DEFINITIONS})

set(QT_LIBRARIES ${Qt5Widgets_LIBRARIES})

add_subdirectory(src)

find_program(Auto_EACSF_LOCATION 
      Auto_EACSF
      HINTS ${CMAKE_CURRENT_BINARY_DIR}/bin)
    if(Auto_EACSF_LOCATION)
      install(PROGRAMS ${Auto_EACSF_LOCATION}
        DESTINATION ${INSTALL_RUNTIME_DESTINATION}
        COMPONENT RUNTIME)
    endif()
set (version "1.0")


find_package(niral_utilities)
if(niral_utilities_FOUND)

  foreach(niral_utilities_lib ${niral_utilities_LIBRARIES})

    get_target_property(niral_utilities_location ${niral_utilities_lib} LOCATION_RELEASE)
    if(NOT EXISTS ${niral_utilities_location})
      message(STATUS "skipping niral_utilities_lib install rule: [${niral_utilities_location}] does not exist")
      continue()
    endif()
    
    if(EXISTS "${niral_utilities_location}.xml")
      install(PROGRAMS ${niral_utilities_location} 
        DESTINATION ${INSTALL_RUNTIME_DESTINATION}
        COMPONENT RUNTIME)

      install(FILES ${niral_utilities_location}.xml
        DESTINATION ${INSTALL_RUNTIME_DESTINATION}
        COMPONENT RUNTIME)
    else()
      install(PROGRAMS ${niral_utilities_location} 
        DESTINATION ${INSTALL_RUNTIME_DESTINATION}
        COMPONENT RUNTIME)      
    endif()
  endforeach()
  
endif()

# find_package(BRAINSCommonLib)
# if(BRAINSCommonLib_FOUND)
#   message ("$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$")
#   message ("######################################")
#   foreach(BRAINSCommonLib_lib ${BRAINSCommonLib_LIBRARIES})
#     message ( ${BRAINSCommonLib_lib})
#     message ("%%%%%%")
#     get_target_property(BRAINSCommonLib_location ${BRAINSCommonLib_lib} LOCATION_RELEASE)
#     if(NOT EXISTS ${BRAINSCommonLib_location})
#       message(STATUS "skipping BRAINSCommonLib_lib install rule: [${BRAINSCommonLib_location}] does not exist")
#       continue()
#     endif()
    
#     if(EXISTS "${BRAINSCommonLib_location}.xml")
#       install(PROGRAMS ${BRAINSCommonLib_location} 
#         DESTINATION ${INSTALL_RUNTIME_DESTINATION}
#         COMPONENT RUNTIME)

#       install(FILES ${BRAINSCommonLib_location}.xml
#         DESTINATION ${INSTALL_RUNTIME_DESTINATION}
#         COMPONENT RUNTIME)
#     else()
#       install(PROGRAMS ${BRAINSCommonLib_location} 
#         DESTINATION ${INSTALL_RUNTIME_DESTINATION}
#         COMPONENT RUNTIME)      
#     endif()
#   endforeach()
#   foreach(BRAINSCommonLib_exe ${BRAINSCommonLib_EXECUTABLES})
#     message(${BRAINSCommonLib_exe})
#   endforeach()
# endif()

if (BRAINSTools_DIR)
  set(BRAINS_tools
    BRAINSFit
    BRAINSResample)
  foreach(BRAINS_bin ${BRAINS_tools})
    find_program(${BRAINS_bin}_LOCATION
      ${BRAINS_bin}
      HINTS ${BRAINSTools_DIR}/bin)
    if(${BRAINS_bin}_LOCATION)
      install(PROGRAMS ${${BRAINS_bin}_LOCATION}
        DESTINATION ${INSTALL_RUNTIME_DESTINATION}
        COMPONENT RUNTIME)
    endif()
  endforeach()
endif()

if(ANTs_DIR)
  set(ants_tools
    ANTS
    WarpImageMultiTransform)
  foreach(ants_bin ${ants_tools})
    find_program(${ants_bin}_LOCATION 
      ${ants_bin}
      HINTS ${ANTs_DIR}/bin)
    if(${ants_bin}_LOCATION)
      install(PROGRAMS ${${ants_bin}_LOCATION}
        DESTINATION ${INSTALL_RUNTIME_DESTINATION}
        COMPONENT RUNTIME)
    endif()
  endforeach()
endif()

if (ABC_DIR)
  find_program(ABC_LOCATION
    ABC_CLI
    HINTS ${ABC_DIR}/StandAloneCLI)
  if(ABC_LOCATION)
    install(PROGRAMS ${ABC_LOCATION}
      DESTINATION ${INSTALL_RUNTIME_DESTINATION}
      COMPONENT RUNTIME)
  endif()
endif()

option(CREATE_BUNDLE "Create a bundle" OFF)

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

  ENDIF(APPLE)

  IF(WIN32)
    SET(APPS "\${CMAKE_INSTALL_PREFIX}/bin/${bundle_name}.exe")
  ENDIF(WIN32)

  

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
  get_target_property(QtCore_location Qt5::Core LOCATION)
  get_target_property(QtGui_location Qt5::Gui LOCATION)
  
  #get_target_property(QtWidgets_SOFT Qt5::Widgets IMPORTED_SONAME_RELEASE)

  get_target_property(Qt5_location Qt5::Widgets LOCATION)  
  string(FIND ${Qt5_location} "libQt5Widgets" length)
  string(SUBSTRING ${Qt5_location} 0 ${length} Qt5_location)
  
  #install(FILES ${QtWidgets_location} ${QtOpenGL_location} ${Qt5_location}/${QtWidgets_SOFT} ${Qt5_location}/${QtOpenGL_SOFT}
  #    DESTINATION lib
  #    COMPONENT Runtime)  

  install(FILES ${QtWidgets_location} ${QtCore_location} ${QtGui_location} ${Qt5_location}/libicui18n.so.56.1 ${Qt5_location}/libicuuc.so.56.1 ${Qt5_location}/libicudata.so.56.1 #${QtOpenGL_location}
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
