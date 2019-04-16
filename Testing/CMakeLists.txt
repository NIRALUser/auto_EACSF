enable_testing()

add_test(NAME computecsfdensityHelp COMMAND $<TARGET_FILE:computecsfdensity>
    --help
  )

# Test checking that the version can be printed even if no input file is given.
add_test(NAME computecsfdensityVersion COMMAND $<TARGET_FILE:computecsfdensity>
    --version
)

include(ExternalData)

set(ExternalData_URL_TEMPLATES
    "file://$ENV{HOME}/Documents/%(algo)/%(hash)"
)
ExternalData_Add_Test(MyData
  NAME computecsfdensityData
  COMMAND $<TARGET_FILE:computecsfdensity> -t DATA{blbltest.txt}
  )
ExternalData_Add_Target(MyData)

#add_dependencies(computecsfdensityData MyData)
