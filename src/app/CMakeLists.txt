set(APP_NAME Auto_EACSF_app)

set(${APP_NAME}_moc csfwindow.h)
set(${APP_NAME}_ui csfwindow.ui)
set(${APP_NAME}_rcc AutoEACSF_Resources.qrc)

qt5_wrap_cpp(${APP_NAME}_generated_moc ${${APP_NAME}_moc})
qt5_wrap_ui(${APP_NAME}_generated_ui ${${APP_NAME}_ui})
qt5_add_resources(${APP_NAME}_generated_rcc ${${APP_NAME}_rcc})

list(APPEND ${APP_NAME}_src
csfwindow.cxx
csfwindow.h
extexecutableswidget.cxx
extexecutableswidget.h
automaticinterface.cxx
automaticinterface.h
${${APP_NAME}_generated_moc}
${${APP_NAME}_generated_ui}
${${APP_NAME}_generated_rcc}
)

add_library(csflib ${${APP_NAME}_src})

add_executable(Auto_EACSF Auto_EACSF.cxx)
target_link_libraries(Auto_EACSF csflib ${QT_LIBRARIES})

add_custom_command(TARGET Auto_EACSF POST_BUILD
                   COMMAND ${CMAKE_COMMAND} -E copy_directory
${CMAKE_SOURCE_DIR}/data ${CMAKE_BINARY_DIR})



