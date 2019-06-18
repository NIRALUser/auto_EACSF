Project(auto_EACSF)
cmake_minimum_required(VERSION 2.8.11)

include(${CMAKE_CURRENT_SOURCE_DIR}/Common.cmake)

option( auto_EACSF_SuperBuild
        "Build auto_EACSF as a Superbuild project" ON )
if( auto_EACSF_SuperBuild )
  include("${CMAKE_CURRENT_SOURCE_DIR}/SuperBuild.cmake")
else()
  include("${CMAKE_CURRENT_SOURCE_DIR}/auto_EACSF.cmake")
endif()
