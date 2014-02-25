SET(SOURCE @SOURCE@)

add_definitions(-DHAVE_CONFIG_H)
add_definitions(-D_ENABLE_HDF5_)

set(NEUROLABI_DIR ../..)
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

add_library(neutube ${SOURCE})



