add_subdirectory(app)

option(COMPILE_COMPUTECSFDENSITY "Compile computeCSFdensity tool." ON)
if(COMPILE_COMPUTECSFDENSITY)
  add_subdirectory(computeCSFdensitylib)
endif()
