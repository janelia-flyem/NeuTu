cmake_minimum_required(VERSION 2.8)

set(SOURCE @SOURCE@)

add_definitions(-DHAVE_CONFIG_H)
add_definitions(-D_ENABLE_HDF5_)

set(NEUROLABI_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../..)
set(EXTLIB_DIR ${NEUROLABI_DIR}/lib)

include_directories(
  ${NEUROLABI_DIR}/gui 
  ${NEUROLABI_DIR}/c 
  ${NEUROLABI_DIR}/c/include 
  ${NEUROLABI_DIR}/gui/ext 
  ${NEUROLABI_DIR}/gui 
  ${EXTLIB_DIR}/genelib/src
  ${EXTLIB_DIR}/xml/include/libxml2
  ${EXTLIB_DIR}/fftw3/include
  ${EXTLIB_DIR}/png/include
  ${EXTLIB_DIR}/jansson/include
  ${EXTLIB_DIR}/hdf5/include
)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fPIC")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC")

#set(neurolabi ${NEUROLABI_DIR}/c/lib/libneurolabi.a)
#set(jansson ${EXTLIB_DIR}/jannson/lib/libjansson.a)
#set(hdf5 ${EXTLIB_DIR}/hdf5/lib/libhdf5.a)
#set(fftw3 ${EXTLIB_DIR}/fftw3/lib/libfftw3.a)
#set(fftw3f ${EXTLIB_DIR}/fftw3/lib/libfftw3f.a)
#set(xml ${EXTLIB_DIR}/xml/lib/libxml2.a)
#set(png ${EXTLIB_DIR}/png/lib/libpng.a)

#find_library(neurolabi LIBRARY NAMES neurolabi PATHS ${NEUROLABI_DIR}/c/lib)
#message("Neurolabi library: ${neurolabi}")

#find_library(jansson LIBRARY NAMES jansson PATHS ${NEUROLABI_DIR}/lib/jansson/lib)
#message("Json library: ${jansson}")

#find_library(hdf5 LIBRARY NAMES hdf5 PATHS ${NEUROLABI_DIR}/lib/hdf5/lib)
#message("HDF5 library: ${hdf5}")


add_library(neutube ${SOURCE})
#target_link_libraries(neutube ${neurolabi}) #doesn't work



