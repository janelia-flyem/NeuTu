#CMake file for tbar src

SET(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -D_DEBUG_")
SET(NeurolabiDir ${CMAKE_CURRENT_SOURCE_DIR}/../../..)
SET(GuiDir ${NeurolabiDir}/gui)
SET(NeurolabiLibDir ${NeurolabiDir}/c/lib)
SET(NeutubeLibDir ${NeurolabiDir}/cpp/lib/build)
SET(EXTLIB_DIR ${NeurolabiDir}/lib)

#SET(jsonLib jansson)
set(CMAKE_PREFIX_PATH ${EXTLIB_DIR}/jansson)
find_library(jsonLib LIBRARY NAMES jansson)

set(CMAKE_PREFIX_PATH ${EXTLIB_DIR}/fftw3)
find_library(fftw3Lib LIBRARY NAMES fftw3)
find_library(fftw3fLib LIBRARY NAMES fftw3f)
set(CMAKE_PREFIX_PATH ${EXTLIB_DIR}/xml)
find_library(xmlLib LIBRARY NAMES xml2)
set(CMAKE_PREFIX_PATH ${EXTLIB_DIR}/png)
find_library(pngLib LIBRARY NAMES png)
set(CMAKE_PREFIX_PATH ${EXTLIB_DIR}/hdf5)
find_library(hdfLib LIBRARY NAMES hdf5)
set(CMAKE_PREFIX_PATH ${EXTLIB_DIR}/hdf5)
find_library(hdfLtLib LIBRARY NAMES hdf5_hl)
set(CMAKE_PREFIX_PATH ${NeurolabiLibDir})
find_library(neuLib LIBRARY NAMES neurolabi)
set(CMAKE_PREFIX_PATH ${NeutubeLibDir})
find_library(neutubeLib LIBRARY NAMES neutube)
find_library(zLib LIBRARY NAMES z)
find_library(dlLib LIBRARY NAMES dl)

message("Json library: ${jsonLib}")
message("fftw3 library: ${fftw3Lib}")
message("fftw3f library: ${fftw3fLib}")
message("xml library: ${xmlLib}")
message("png library: ${pngLib}")
message("hdf library: ${hdfLib}")
message("hdflt library: ${hdfLtLib}")
message("neurolabi library: ${neuLib}")
message("neutube library: ${neutubeLib}")
message("libz library: ${zLib}")
message("dl library: ${dlLib}")

include_directories(
  ${NeurolabiDir}/c 
  ${NeurolabiDir}/c/include 
  ${NeurolabiDir}/gui/ext 
  ${NeurolabiDir}/gui 
  ${EXTLIB_DIR}/genelib/src
  ${EXTLIB_DIR}/xml/include/libxml2
  ${EXTLIB_DIR}/fftw3/include
  ${EXTLIB_DIR}/png/include
  ${EXTLIB_DIR}/jansson/include
  ${EXTLIB_DIR}/hdf5/include
  )

add_executable(Tbar_apply_sa Tbar_apply_sa.cpp)
TARGET_LINK_LIBRARIES(Tbar_apply_sa 
  ${neutubeLib} ${neuLib} ${jsonLib} ${fftw3Lib} ${fftw3fLib} ${xmlLib} 
  ${pngLib} ${hdfLtLib} ${hdfLib} ${zLib} ${dlLib} 
  )



