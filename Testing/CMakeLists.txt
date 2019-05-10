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
    "file://$ENV{HOME}/Documents/md5/a75b2abfc0681e8e95bf418db6469a5d"
)
ExternalData_Add_Test(MyData
  NAME Auto_EACSF_Data
  COMMAND $<TARGET_FILE:Auto_EACSF> -t DATA{external_file.txt}
  )
ExternalData_Add_Target(MyData)

#add_dependencies(computecsfdensityData MyData)
