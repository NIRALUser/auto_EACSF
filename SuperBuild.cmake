set(PRIMARY_PROJECT_NAME auto_EACSF)

set( ${PRIMARY_PROJECT_NAME}_USE_QT ON )

set(EXTERNAL_SOURCE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR} CACHE PATH "Select where external packages will be downloaded" )
set(EXTERNAL_BINARY_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR} CACHE PATH "Select where external packages will be compiled and installed" )

option(EXECUTABLES_ONLY "Build the tools and the tools' libraries statically" ON)

#------------------------------------------------------------------------------
# ${PRIMARY_PROJECT_NAME} dependency list
#------------------------------------------------------------------------------
set(${PRIMARY_PROJECT_NAME}_DEPENDENCIES VTK ITKv4 Python3 SlicerExecutionModel ABC ANTs BRAINSTools niral_utilities)

option(USE_SYSTEM_ITK "Build using an externally defined version of ITK" OFF)
option(USE_SYSTEM_SlicerExecutionModel "Build using an externally defined version of SlicerExecutionModel"  OFF)
option(USE_SYSTEM_VTK "Build using an externally defined version of VTK" OFF)
option(USE_SYSTEM_ANTs "Build using an externally defined version of ANTS" OFF)

#-----------------------------------------------------------------------------
# Define Superbuild global variables
#-----------------------------------------------------------------------------

# This variable will contain the list of CMake variable specific to each external project
# that should passed to ${CMAKE_PROJECT_NAME}.
# The item of this list should have the following form: <EP_VAR>:<TYPE>
# where '<EP_VAR>' is an external project variable and TYPE is either BOOL, STRING, PATH or FILEPATH.
# TODO Variable appended to this list will be automatically exported in ${PRIMARY_PROJECT_NAME}Config.cmake,
# prefix '${PRIMARY_PROJECT_NAME}_' will be prepended if it applies.
set(${CMAKE_PROJECT_NAME}_SUPERBUILD_EP_VARS)

# The macro '_expand_external_project_vars' can be used to expand the list of <EP_VAR>.
set(${CMAKE_PROJECT_NAME}_SUPERBUILD_EP_ARGS) # List of CMake args to configure BRAINS
set(${CMAKE_PROJECT_NAME}_SUPERBUILD_EP_VARNAMES) # List of CMake variable names

# Convenient macro allowing to expand the list of EP_VAR listed in ${CMAKE_PROJECT_NAME}_SUPERBUILD_EP_VARS
# The expanded arguments will be appended to the list ${CMAKE_PROJECT_NAME}_SUPERBUILD_EP_ARGS
# Similarly the name of the EP_VARs will be appended to the list ${CMAKE_PROJECT_NAME}_SUPERBUILD_EP_VARNAMES.
macro(_expand_external_project_vars)
  set(${CMAKE_PROJECT_NAME}_SUPERBUILD_EP_ARGS "")
  set(${CMAKE_PROJECT_NAME}_SUPERBUILD_EP_VARNAMES "")
  foreach(arg ${${CMAKE_PROJECT_NAME}_SUPERBUILD_EP_VARS})
    string(REPLACE ":" ";" varname_and_vartype ${arg})
    set(target_info_list ${target_info_list})
    list(GET varname_and_vartype 0 _varname)
    list(GET varname_and_vartype 1 _vartype)
    list(APPEND ${CMAKE_PROJECT_NAME}_SUPERBUILD_EP_ARGS -D${_varname}:${_vartype}=${${_varname}})
    list(APPEND ${CMAKE_PROJECT_NAME}_SUPERBUILD_EP_VARNAMES ${_varname})
  endforeach()
endmacro()



#-----------------------------------------------------------------------------
# Common external projects CMake variables
#-----------------------------------------------------------------------------
list(APPEND ${CMAKE_PROJECT_NAME}_SUPERBUILD_EP_VARS
  MAKECOMMAND:STRING
  CMAKE_SKIP_RPATH:BOOL
  CMAKE_BUILD_TYPE:STRING
  BUILD_SHARED_LIBS:BOOL
  CMAKE_CXX_COMPILER:PATH
  CMAKE_CXX_FLAGS_RELEASE:STRING
  CMAKE_CXX_FLAGS_DEBUG:STRING
  CMAKE_CXX_FLAGS:STRING
  CMAKE_C_COMPILER:PATH
  CMAKE_C_FLAGS_RELEASE:STRING
  CMAKE_C_FLAGS_DEBUG:STRING
  CMAKE_C_FLAGS:STRING
  CMAKE_SHARED_LINKER_FLAGS:STRING
  CMAKE_MODULE_LINKER_FLAGS:STRING
  CMAKE_EXE_LINKER_FLAGS:STRING
  CMAKE_GENERATOR:STRING
  CMAKE_EXTRA_GENERATOR:STRING
  CMAKE_INSTALL_PREFIX:PATH
  CMAKE_LIBRARY_OUTPUT_DIRECTORY:PATH
  CMAKE_ARCHIVE_OUTPUT_DIRECTORY:PATH
  CMAKE_RUNTIME_OUTPUT_DIRECTORY:PATH
  CMAKE_BUNDLE_OUTPUT_DIRECTORY:PATH
  CTEST_NEW_FORMAT:BOOL
  MEMORYCHECK_COMMAND_OPTIONS:STRING
  MEMORYCHECK_COMMAND:PATH
  SITE:STRING
  BUILDNAME:STRING
  Subversion_SVN_EXECUTABLE:FILEPATH
  GIT_EXECUTABLE:FILEPATH
  USE_GIT_PROTOCOL:BOOL
  )

_expand_external_project_vars()
set(COMMON_EXTERNAL_PROJECT_ARGS ${${CMAKE_PROJECT_NAME}_SUPERBUILD_EP_ARGS})
set(extProjName ${PRIMARY_PROJECT_NAME})
set(proj        ${PRIMARY_PROJECT_NAME})
SlicerMacroCheckExternalProjectDependency(${PRIMARY_PROJECT_NAME})

#-----------------------------------------------------------------------------
# Set CMake OSX variable to pass down the external project
#-----------------------------------------------------------------------------
set(CMAKE_OSX_EXTERNAL_PROJECT_ARGS)
if(APPLE)
  list(APPEND CMAKE_OSX_EXTERNAL_PROJECT_ARGS
    -DCMAKE_OSX_ARCHITECTURES=${CMAKE_OSX_ARCHITECTURES}
    -DCMAKE_OSX_SYSROOT=${CMAKE_OSX_SYSROOT}
    -DCMAKE_OSX_DEPLOYMENT_TARGET=${CMAKE_OSX_DEPLOYMENT_TARGET})
endif()


#-----------------------------------------------------------------------------
# Add external project CMake args
#-----------------------------------------------------------------------------
list(APPEND ${CMAKE_PROJECT_NAME}_SUPERBUILD_EP_VARS
  BUILD_TESTING:BOOL
  EXTENSION:BOOL
  CMAKE_MODULE_PATH:PATH
  EXTENSION_NAME:STRING
  MIDAS_PACKAGE_EMAIL:STRING
  MIDAS_PACKAGE_API_KEY:STRING
  )

_expand_external_project_vars()
set(COMMON_EXTERNAL_PROJECT_ARGS ${${CMAKE_PROJECT_NAME}_SUPERBUILD_EP_ARGS})


if(verbose)
  message("Inner external project args:")
  foreach(arg ${${CMAKE_PROJECT_NAME}_SUPERBUILD_EP_ARGS})
    message("  ${arg}")
  endforeach()
endif()

string(REPLACE ";" "^" ${CMAKE_PROJECT_NAME}_SUPERBUILD_EP_VARNAMES "${${CMAKE_PROJECT_NAME}_SUPERBUILD_EP_VARNAMES}")

if(verbose)
  message("Inner external project argnames:")
  foreach(argname ${${CMAKE_PROJECT_NAME}_SUPERBUILD_EP_VARNAMES})
    message("  ${argname}")
  endforeach()
endif()

set(proj ${PRIMARY_PROJECT_NAME}-inner)
set(${proj}_INSTALL_PATH "${CMAKE_CURRENT_BINARY_DIR}/${proj}-install")
ExternalProject_Add(${proj}
  DEPENDS ${${PRIMARY_PROJECT_NAME}_DEPENDENCIES}
  DOWNLOAD_COMMAND ""  
  SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}
  BINARY_DIR ${proj}-build
  CMAKE_GENERATOR ${gen}
  CMAKE_ARGS
    -DCMAKE_INSTALL_PREFIX:PATH=${${proj}_INSTALL_PATH}
    -Dauto_EACSF_SuperBuild:BOOL=OFF
    -DINSTALL_RUNTIME_DESTINATION:PATH=${INSTALL_RUNTIME_DESTINATION}
    -DINSTALL_LIBRARY_DESTINATION:PATH=${INSTALL_LIBRARY_DESTINATION}
    -DQt5_DIR:PATH=${Qt5_DIR}
    -DCMAKE_CXX_FLAGS:STRING=-std=c++11 -fPIC
    -DITK_DIR:PATH=${ITK_DIR}
    -DVTK_DIR:PATH=${VTK_DIR}
    -DSlicerExecutionModel_DIR:PATH=${SlicerExecutionModel_DIR}
    -DANTs_DIR:PATH=${ANTs_DIR}
    -Dniral_utilities_DIR:PATH=${niral_utilities_DIR}
    -DABC_DIR:PATH=${ABC_DIR}
    -DBRAINSTools_DIR:PATH=${BRAINSTools_DIR}
    ${CMAKE_OSX_EXTERNAL_PROJECT_ARGS}
)
