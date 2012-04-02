# Search for fftw3 headers
INCLUDE(FindPackageMessage)

FIND_PATH(FFTW_INCLUDE_DIR fftw3.h  ${FFTW_INCLUDE_DIRS})
FIND_LIBRARY(FFTW_LIBRARY NAMES fftw3 ${FFTW_LIBRARY_DIRS})
FIND_LIBRARY(FFTWF_LIBRARY NAMES fftw3f ${FFTW_LIBRARY_DIRS})
SET(FFTW_LIBRARIES ${FFTW_LIBRARY} ${FFTWF_LIBRARY})

SET(FFTW_FOUND FALSE)
IF(FFTW_INCLUDE_DIR AND FFTW_LIBRARY AND FFTWF_LIBRARY)
  FIND_PACKAGE_MESSAGE(FFTW "Found FFTW: ${FFTW_LIBRARIES}" "[${FFTW_LIBRARIES}][${FFTW_INCLUDE_DIR}]")
  SET(FFTW_FOUND TRUE)
ENDIF()

MARK_AS_ADVANCED(
   FFTW_INCLUDE_DIR
   FFTW_LIBRARIES
   FFTW_FOUND
)