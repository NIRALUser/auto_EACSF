add_subdirectory(app)

option(COMPILE_KLAPLACE "Compile klaplace." ON)
if(COMPILE_KLAPLACE)
  add_subdirectory(klaplacelib)
endif()