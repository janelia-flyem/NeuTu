cmake_minimum_required(VERSION 2.8)

if (NOT CMAKE_BUILD_TYPE)
  set (CMAKE_BUILD_TYPE "Debug" CACHE STRING
      "Choose the type of build, options are: Debug Release
RelWithDebInfo MinSizeRel."
      FORCE)
  message ("Default to Debug build, use gui to change CMAKE_BUILD_TYPE or
use -DCMAKE_BUILD_TYPE=Release in command line. Options are: Debug Release
RelWithDebInfo MinSizeRel.")
endif (NOT CMAKE_BUILD_TYPE)

project(neurolabi)

set (USE_LIBFFTW ON CACHE BOOL "Use fftw3")
set (USE_LIBFFTWF ON CACHE BOOL "Use float version of fftw")
set (USE_LIBZ ON CACHE BOOL "Use libz")
set (USE_LIBXML2 ON CACHE BOOL "Use libxml2")
set (USE_LIBPNG OFF CACHE BOOL "Use libpng")
set (USE_LIBJANSSON ON CACHE BOOL "Use libjansson")

set (CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake_modules)

include (CheckIncludeFiles)
include (CheckLibraryExists)
include (CheckFunctionExists)
include (CheckTypeSize)
include (CheckCInlineKeyword)

#check arch bit
if (CMAKE_SIZE_OF_VOID_P EQUAL 4)
  set (ARCH_BIT 32)
else()
  set (ARCH_BIT 64)
endif()
add_definitions(-DARCH_BIT=${ARCH_BIT})

if (USE_LIBFFTW)
  find_package(FFTW3 REQUIRED)
  include_directories(${FFTW3_INCLUDE_DIRS})
  set(EXT_LIBS ${EXT_LIBS} ${FFTW3_LIBRARIES})
  check_library_exists(${FFTW3_LIBRARIES} fftw_execute "" HAVE_LIBFFTW3)
endif (USE_LIBFFTW)

if (USE_LIBFFTWF)
  find_package(FFTW3F REQUIRED)
  include_directories(${FFTW3F_INCLUDE_DIRS})
  set(EXT_LIBS ${EXT_LIBS} ${FFTW3F_LIBRARIES})
  check_library_exists(${FFTW3F_LIBRARIES} fftwf_execute "" HAVE_LIBFFTW3F)
endif (USE_LIBFFTWF)

if (USE_LIBZ)
  if (WIN32)
    find_package(ZLIB_MY REQUIRED)
  else (WIN32)
    find_package(ZLIB REQUIRED)
  endif (WIN32)
  include_directories(${ZLIB_INCLUDE_DIRS})
  set(EXT_LIBS ${EXT_LIBS} ${ZLIB_LIBRARIES})
  check_library_exists(${ZLIB_LIBRARIES} compress "" HAVE_LIBZ)
endif (USE_LIBZ)

if (USE_LIBXML2)
  find_package(LibXml2_MY REQUIRED)
  include_directories(${LIBXML2_INCLUDE_DIRS})
  set(EXT_LIBS ${EXT_LIBS} ${LIBXML2_LIBRARIES})
  #check_library_exists(${LIBXML2_LIBRARIES} xmlParseFile "" HAVE_LIBXML2)
  set(HAVE_LIBXML2 TRUE)
endif (USE_LIBXML2)

if (USE_LIBPNG)
  find_package(PNG_MY REQUIRED)
  include_directories(${PNG_INCLUDE_DIRS})
  set(EXT_LIBS ${EXT_LIBS} ${PNG_LIBRARIES})
  #check_library_exists(${PNG_LIBRARIES} png_read_image "" HAVE_LIBPNG)
  set(HAVE_LIBPNG TRUE)
endif(USE_LIBPNG)

if (USE_LIBJANSSON)
  find_package(JANSSON REQUIRED)
  include_directories(${JANSSON_INCLUDE_DIRS})
  set(EXT_LIBS ${EXT_LIBS} ${JANSSON_LIBRARIES})
  check_library_exists(${JANSSON_LIBRARIES} json_object "" HAVE_LIBJANSSON)
endif(USE_LIBJANSSON)

check_include_files (stddef.h HAVE_STDDEF_H)
check_include_files (stdint.h HAVE_STDINT_H)
check_include_files (stdlib.h HAVE_STDLIB_H)
check_include_files (string.h HAVE_STRING_H)
check_include_files (strings.h HAVE_STRINGS_H)
check_include_files (sys/time.h HAVE_SYS_TIME_H)
check_include_files (unistd.h HAVE_UNISTD_H)
check_include_files (regex.h HAVE_REGEX_H)
check_include_files (dirent.h HAVE_DIRENT_H)

# use pcreposix if can not find regex.h
if (NOT HAVE_REGEX_H)
  find_package(PCRE REQUIRED)
  include_directories(${PCRE_INCLUDE_DIRS})
  set(EXT_LIBS ${EXT_LIBS} ${PCRE_LIBRARIES})
endif (NOT HAVE_REGEX_H)

check_function_exists(bzero HAVE_BZERO)
check_function_exists(floor HAVE_FLOOR)
check_function_exists(round HAVE_ROUND)
check_function_exists(lrint HAVE_LRNT)
check_function_exists(gettimeofday HAVE_GETTIMEOFDAY)
check_function_exists(memmove HAVE_MEMMOVE)
check_function_exists(memset HAVE_MEMSET)
check_function_exists(regcomp HAVE_REGCOMP)
check_function_exists(sqrt HAVE_SQRT)
check_function_exists(memset_pattern4 HAVE_MEMSET_PATTERN4)
check_function_exists(fgetln HAVE_GETLN)

CHECK_TYPE_SIZE(char SIZEOF_CHAR)
check_type_size(int SIZEOF_INT)
check_type_size(short SIZEOF_SHORT)
check_type_size(long SIZEOF_LONG)

OPTION( INTERFACE_PROGRESS_OFF "Disable Progress bar" ON )

configure_file (${PROJECT_SOURCE_DIR}/include/neurolabi_config_cmake.in
  ${PROJECT_SOURCE_DIR}/include/neurolabi_config.h)


#Set some flags
if (CMAKE_BUILD_TYPE STREQUAL Debug)
  add_definitions (-D_DEBUG_ -D_TEST_ -DHAVE_CONFIG_H -DLIBMYERS_VERSION=2)
else (CMAKE_BUILD_TYPE STREQUAL Debug)
  add_definitions (-DHAVE_CONFIG_H -DLIBMYERS_VERSION=2)
endif (CMAKE_BUILD_TYPE STREQUAL Debug)
if (MINGW)
  add_definitions (-D__USE_MINGW_ANSI_STDIO)
  add_definitions (-DLIBXML_STATIC)
endif (MINGW)

if (MSVC)
  set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /W4 /D \"_CRT_SECURE_NO_WARNINGS\"")
  set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4 /D \"_CRT_SECURE_NO_WARNINGS\"")
else (MSVC)
  set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall")
endif (MSVC)

if (APPLE)
  if (CMAKE_SIZEOF_VOID_P EQUAL 8)
    set (CMAKE_OSX_ARCHITECTURES x86_64)
  else (CMAKE_SIZEOF_VOID_P EQUAL 8)
    set (CMAKE_OSX_ARCHITECTURES i386)
  endif (CMAKE_SIZEOF_VOID_P EQUAL 8)
elseif (CMAKE_COMPILER_IS_GNUCC OR MINGW)
  if (CMAKE_SIZEOF_VOID_P EQUAL 8)
    set (CMAKE_C_FLAGS  "${CMAKE_C_FLAGS} -m64")
  else (CMAKE_SIZEOF_VOID_P EQUAL 8)
    set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -m32")
  endif (CMAKE_SIZEOF_VOID_P EQUAL 8)
endif (APPLE)

#Set sources
set (GENELIB_DIR  ${CMAKE_CURRENT_SOURCE_DIR}/../lib/genelib/src)
include (${GENELIB_DIR}/genelib.cmake)
source_group("" FILES ${GENELIB_SRCS})
source_group(generated FILES ${GENERATED_GENELIB_SRCS})

# neulib srcs generated from template
include (parse_template.cmake)
parse_all_template ()

# other neulib srcs
set (NEULIB_SRCS
  tz_image_lib.c tz_math_lib.c tz_complex.c
  tz_utilities.c tz_matlabio.c tz_array.c tz_error.c
  tz_complex_f.c tz_mxutils.c tz_string.c tz_pipe.c tz_arraytrans.c
  tz_image_code.c tz_3dgeom.c tz_object_tmpl.c tz_stack_bwmorph.c
  tz_neuroseg.c tz_neuropos.c tz_object_3d.c
  tz_objdetect.c tz_pixel_array.c tz_stack_draw.c
  tz_stack_math.c tz_image_io.c tz_object_3d_linked_list_ext.c
  tz_stack_threshold.c tz_stack_attribute.c tz_stack_relation.c
  tz_object_3d_operation.c tz_geometry.c tz_stack_lib.c
  tz_stack_stat.c tz_voxel_graphics.c
  tz_voxel.c tz_voxeltrans.c tz_geo3d_vector.c tz_tvoxel.c
  tz_stack_sampling.c tz_stack_utils.c
  tz_local_neuroseg.c tz_neurofield.c tz_geo3d_utils.c tz_bitmask.c
  tz_geoangle_utils.c tz_stack_neighborhood.c tz_stack_objlabel.c
  tz_gsl_linalg.c tz_optimize_utils.c tz_arrayview.c
  tz_image_attribute.c tz_image_relation.c
  tz_image_array.c tz_int_histogram.c tz_stack_io.c tz_interface.c
  tz_coordinate_3d.c tz_geo3d_point_array.c tz_stack_objmask.c
  tz_vrml_io.c tz_geo3d_circle.c tz_xz_orientation.c
  tz_vrml_material.c tz_perceptor.c tz_cont_fun.c
  tz_geo3d_scalar_field.c tz_neuroseg_plane.c
  tz_local_neuroseg_plane.c tz_bifold_neuroseg.c
  tz_local_bifold_neuroseg.c tz_vrml_utils.c tz_locnp_chain.c
  tz_neuroseg_ellipse.c tz_local_neuroseg_ellipse.c tz_unipointer.c
  tz_locne_chain.c tz_graph.c tz_graph_workspace.c tz_arrayqueue.c
  tz_graph_utils.c tz_voxel_linked_list_ext.c tz_locseg_chain.c
  tz_locseg_chain_exp.c tz_trace_utils.c tz_stack.c tz_stack_graph.c
  tz_neuron_component.c tz_neuron_structure.c tz_swc_cell.c
  tz_color.c tz_neurocomp_conn.c tz_random.c
  tz_math.c tz_mc_stack.c tz_image_trans.c tz_string_workspace.c
  tz_locseg_chain_network.c tz_rastergeom.c tz_tiff_image.c
  tz_geo3d_ball.c tz_intpair_map.c tz_workspace.c tz_xml_utils.c
  tz_stack_document.c tz_swc.c tz_swc_tree.c
  tz_locseg_chain_knot.c tz_geo3d_ellipse.c tz_stack_tile.c
  tz_sp_grow.c tz_stack_watershed.c tz_geo3d_cuboid.c tz_cuboid_i.c
  tz_stack_tile_i.c tz_testdata.c
  tz_local_rpi_neuroseg.c tz_rpi_neuroseg.c tz_receptor_transform.c
  tz_r2_rect.c tz_r2_ellipse.c
  tz_apo.c tz_png_io.c tz_file_list.c tz_json.c tz_array.c.t tz_fftw.h.t
  tz_timage_lib.h.t tz_array.h.t tz_linked_list.c.t tz_trace_chain_com.c.t
  tz_arraylist.c.t tz_linked_list.h.t tz_trace_chain_com.h.t
  tz_arraylist.h.t tz_matrix.a.t tz_trace_node.c.t
  tz_doubly_linked_list.c.t tz_matrix.c.t tz_trace_node.h.t
  tz_doubly_linked_list.h.t tz_matrix.h.t tz_fftw.c.t tz_timage_lib.c.t)

source_group("" FILES ${NEULIB_SRCS})
source_group(generated FILES ${GENERATED_NEULIB_SRCS})

INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/include ${GENELIB_DIR})

#Setup targets
if(CMAKE_BUILD_TYPE STREQUAL Debug)
  set(LIBRARY_NAME neurolabi_debug)
else(CMAKE_BUILD_TYPE STREQUAL Debug)
  set(LIBRARY_NAME neurolabi)
endif(CMAKE_BUILD_TYPE STREQUAL Debug)
message("library name: ${LIBRARY_NAME}")

add_library(${LIBRARY_NAME} STATIC ${NEULIB_SRCS} ${GENERATED_NEULIB_SRCS}
  ${GENELIB_SRCS} ${GENERATED_GENELIB_SRCS})

# install library
install (TARGETS ${LIBRARY_NAME} ARCHIVE DESTINATION ${PROJECT_SOURCE_DIR}/lib)

set (CPACK_PACKAGE_VERSION "1.0.0")

include (CPack)




