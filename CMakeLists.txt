project(simpleSTORM)
cmake_minimum_required(VERSION 2.8)
SET(CMAKE_MODULE_PATH  ${CMAKE_MODULE_PATH}  ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

FIND_PACKAGE( Vigra 1.9.0 REQUIRED )
FIND_PACKAGE( FFTW REQUIRED )
FIND_PACKAGE( HDF5 )
FIND_PACKAGE( OpenMP )
FIND_PACKAGE( R REQUIRED)
FIND_PACKAGE( RudeConfig REQUIRED)

IF(HDF5_FOUND)
    ADD_DEFINITIONS(-DHDF5_FOUND)
ELSE(HDF5_FOUND)
    message(WARNING "Compiling without HDF5. No hdf5-input will be possible")
ENDIF(HDF5_FOUND)


IF(HDF5_FOUND)
    ADD_DEFINITIONS(-DHDF5_FOUND)
ELSE(HDF5_FOUND)
    message(WARNING "Compiling without HDF5. No hdf5-input will be possible")
ENDIF(HDF5_FOUND)


include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/storm
    ${Vigra_INCLUDE_DIRS}
    ${FFTW_INCLUDE_DIR}
    ${R_INCLUDE_DIR}
    ${RUDECONFIG_INCLUDE_DIR}
)

set(CMAKE_CXX_FLAGS
    "${CMAKE_CXX_FLAGS} -std=c++11"
)

set(BIN_INSTALL_DIR bin)

if (CMAKE_CXX_COMPILER_ID STREQUAL Clang)
   set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++")
endif (CMAKE_CXX_COMPILER_ID STREQUAL Clang)

add_subdirectory(storm)
add_subdirectory(gui)
