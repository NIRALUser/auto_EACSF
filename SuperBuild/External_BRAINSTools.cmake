# Make sure this file is included only once
get_filename_component(CMAKE_CURRENT_LIST_FILENAME ${CMAKE_CURRENT_LIST_FILE} NAME_WE)
if(${CMAKE_CURRENT_LIST_FILENAME}_FILE_INCLUDED)
  return()
endif()
set(${CMAKE_CURRENT_LIST_FILENAME}_FILE_INCLUDED 1)

if( NOT EXTERNAL_BINARY_DIRECTORY )
  set( EXTERNAL_BINARY_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR} )
endif()

# Sanity checks
if(DEFINED BRAINSTools_SOURCE_DIR AND NOT EXISTS ${BRAINSTools_SOURCE_DIR})
  message(FATAL_ERROR "BRAINSTools_SOURCE_DIR variable is defined but corresponds to non-existing directory")
endif()

# Set dependency list
set(BRAINSTools_DEPENDENCIES ${ITK_EXTERNAL_NAME} SlicerExecutionModel VTK)

# Include dependent projects if any
SlicerMacroCheckExternalProjectDependency(BRAINSTools)
set(proj BRAINSTools)

# Set CMake OSX variable to pass down the external project
set(CMAKE_OSX_EXTERNAL_PROJECT_ARGS)
if(APPLE)
  list(APPEND CMAKE_OSX_EXTERNAL_PROJECT_ARGS
    -DCMAKE_OSX_ARCHITECTURES=${CMAKE_OSX_ARCHITECTURES}
    -DCMAKE_OSX_SYSROOT=${CMAKE_OSX_SYSROOT}
    -DCMAKE_OSX_DEPLOYMENT_TARGET=${CMAKE_OSX_DEPLOYMENT_TARGET})
endif()

if(NOT DEFINED BRAINSTools_SOURCE_DIR)

  set(GIT_TAG "v4.3.0")

  if(NOT DEFINED git_protocol)
    set(git_protocol "git")
  endif()

  ExternalProject_Add(${proj}
    GIT_REPOSITORY "https://github.com/BRAINSia/BRAINSTools.git"
    GIT_TAG "${GIT_TAG}"
    SOURCE_DIR ${proj}
    BINARY_DIR ${proj}-build
    CMAKE_GENERATOR ${gen}
    "${cmakeversion_external_update}"
    CMAKE_ARGS
      -Wno-dev
      --no-warn-unused-cli
      ${CMAKE_OSX_EXTERNAL_PROJECT_ARGS}
      ${COMMON_EXTERNAL_PROJECT_ARGS}
      -DCMAKE_INSTALL_PREFIX:PATH=${EXTERNAL_BINARY_DIRECTORY}/${proj}-install
      -D${proj}_SUPERBUILD:BOOL=OFF
      -DBUILD_EXAMPLES:BOOL=OFF
      -DBUILD_TESTING:BOOL=OFF
      -DUSE_SYSTEM_ITK:BOOL=ON
      -DUSE_SYSTEM_VTK:BOOL=ON
      -DUSE_SYSTEM_SlicerExecutionModel:BOOL=ON
      -DITK_DIR:PATH=${ITK_DIR}
      -DVTK_DIR:PATH=${VTK_DIR}
      -DUSE_ANTS:BOOL=OFF
      -DUSE_AutoWorkup:BOOL=OFF
      -DUSE_BRAINSABC:BOOL=OFF
      -DUSE_BRAINSConstellationDetector:BOOL=OFF
      -DUSE_BRAINSContinuousClass:BOOL=OFF
      -DUSE_BRAINSCreateLabelMapFromProbabilityMaps=OFF
      -DUSE_BRAINSCut:BOOL=OFF
      -DUSE_BRAINSDemonWarp:BOOL=OFF
      -DUSE_BRAINSDWICleanup=OFF
      -DUSE_BRAINSFit:BOOL=ON
      -DUSE_BRAINSFitEZ:BOOL=OFF
      -DUSE_BRAINSImageConvert:BOOL=OFF
      -DUSE_BRAINSInitializedControlPoints:BOOL=OFF
      -DUSE_BRAINSLabelStats=OFF
      -DUSE_BRAINSLandmarkInitializer:BOOL=OFF
      -DUSE_BRAINSMultiModeSegment:BOOL=OFF
      -DUSE_BRAINSMultiSTAPLE=OFF
      -DUSE_BRAINSMush:BOOL=OFF
      -DUSE_BRAINSPosteriorToContinuousClass=OFF
      -DUSE_BRAINSROIAuto:BOOL=OFF
      -DUSE_BRAINSResample:BOOL=ON
      -DUSE_BRAINSSnapShotWriter:BOOL=OFF
      -DUSE_BRAINSSurfaceTools:BOOL=OFF
      -DUSE_BRAINSStripRotation=OFF
      -DUSE_BRAINSTalairach=OFF
      -DUSE_BRAINSTransformConvert:BOOL=OFF
      -DUSE_BRAINSTransformConvert=OFF
      -DUSE_ConvertBetweenFileFormats=OFF
      -DUSE_DWIConvert=OFF
      -DUSE_DebugImageViewer=OFF
      -DUSE_ReferenceAtlas=OFF
      -DUSE_DebugImageViewer:BOOL=OFF
      -DUSE_GTRACT:BOOL=OFF
      -DUSE_ICCDEF:BOOL=OFF
      -DUSE_ImageCalculator:BOOL=OFF
      -DSlicerExecutionModel_DIR:PATH=${SlicerExecutionModel_DIR}
      -DPYTHON_EXECUTABLE:PATH=${Python3_EXECUTABLE}
      -DPYTHON_LIBRARIES:PATH=${Python3_LIBRARIES}
      ${${proj}_CMAKE_OPTIONS}
    DEPENDS
      ${BRAINSTools_DEPENDENCIES}
    )
  set(BRAINSTools_SOURCE_DIR ${CMAKE_BINARY_DIR}/${proj})
  set(BRAINSCommonLib_DIR    ${CMAKE_BINARY_DIR}/${proj}-build/BRAINSTools-build/BRAINSCommonLib)
  set(${proj}_DIR ${CMAKE_BINARY_DIR}/${proj}-install)
else()
  # The project is provided using BRAINSTools_DIR, nevertheless since other project may depend on BRAINSTools,
  # let's add an 'empty' one
  SlicerMacroEmptyExternalProject(${proj} "${BRAINSTools_DEPENDENCIES}")
endif()