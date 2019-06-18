# Find Qwt include directory and libraries
#
# QWT_FOUND - system has Qwt
# QWT_INCLUDE_DIR - where to find qwt.h
# QWT_LIBRARIES - the libraries to link against to use Qwt

IF(NOT QT4_FOUND)
    FIND_PACKAGE(Qt4 REQUIRED)
ENDIF(NOT QT4_FOUND)

SET(QWT_FOUND "NO")

IF(QT4_FOUND)
    FIND_PATH(QWT_INCLUDE_DIR qwt.h
    /usr/local/qwt/include
    /usr/local/include
    /usr/include/qwt
    /usr/include
    )

    SET(QWT_NAMES ${QWT_NAMES} qwt libqwt)
    FIND_LIBRARY(QWT_LIBRARY
        NAMES ${QWT_NAMES}
        PATHS ${QWT_LIBRARY_PATH} /usr/local/qwt/lib /usr/local/lib /usr/lib
    )

    IF (QWT_LIBRARY)
        SET(QWT_LIBRARIES ${QWT_LIBRARY})
        IF (CYGWIN)
            IF(NOT BUILD_SHARED_LIBS)
                SET (QWT_DEFINITIONS -DQWT_STATIC)
            ENDIF(NOT BUILD_SHARED_LIBS)
        ENDIF (CYGWIN)

    ENDIF (QWT_LIBRARY)
ENDIF(QT4_FOUND)

IF (QWT_INCLUDE_DIR AND QWT_LIBRARY)
  SET(QWT_FOUND TRUE)
ENDIF (QWT_INCLUDE_DIR AND QWT_LIBRARY)

IF (QWT_FOUND)
  IF (NOT QWT_FIND_QUIETLY)
    MESSAGE(STATUS "Found Qwt: ${QWT_LIBRARY}")
  ENDIF (NOT QWT_FIND_QUIETLY)
ELSE (QWT_FOUND)
    MESSAGE(FATAL_ERROR "Could not find Qwt library")
ENDIF (QWT_FOUND)
