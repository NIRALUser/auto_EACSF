set(APP_NAME Auto_EACSF_app)

set(${APP_NAME}_moc csfwindow.h)
set(${APP_NAME}_ui csfwindow.ui)
set(${APP_NAME}_rcc AutoEACSF_Resources.qrc)

set_property(SOURCE csfwindow.h PROPERTY SKIP_AUTOMOC ON)

qt5_wrap_cpp(${APP_NAME}_generated_moc ${${APP_NAME}_moc})
qt5_wrap_ui(${APP_NAME}_generated_ui ${${APP_NAME}_ui})
qt5_add_resources(${APP_NAME}_generated_rcc ${${APP_NAME}_rcc})

list(APPEND ${APP_NAME}_src
	    csfwindow.cxx
	    csfwindow.h
	    extexecutableswidget.cxx
	    extexecutableswidget.h
	    ${${APP_NAME}_generated_moc}
	    ${${APP_NAME}_generated_ui}
	    ${${APP_NAME}_generated_rcc}
)

#add_library(csflib ${${APP_NAME}_src})

find_package(SlicerExecutionModel REQUIRED)
include(${SlicerExecutionModel_USE_FILE})

SEMMacroBuildCLI(
  NAME Auto_EACSF
  EXECUTABLE_ONLY
  TARGET_LIBRARIES ${QT_LIBRARIES}
  ADDITIONAL_SRCS ${${APP_NAME}_src}
  RUNTIME_OUTPUT_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
  LIBRARY_OUTPUT_DIRECTORY ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}
  ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_ARCHIVE_OUTPUT_DIRECTORY}
  INSTALL_RUNTIME_DESTINATION ${INSTALL_RUNTIME_DESTINATION}
  INSTALL_LIBRARY_DESTINATION ${INSTALL_LIBRARY_DESTINATION}
  INSTALL_ARCHIVE_DESTINATION ${INSTALL_ARCHIVE_DESTINATION}
)

#add_executable(Auto_EACSF Auto_EACSF.cxx)
#target_link_libraries(Auto_EACSF csflib ${QT_LIBRARIES})

#add_custom_command(TARGET Auto_EACSF POST_BUILD
#                   COMMAND ${CMAKE_COMMAND} -E copy_directory
#${CMAKE_SOURCE_DIR}/data ${CMAKE_BINARY_DIR})



