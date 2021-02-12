cmake_minimum_required(VERSION 3.8)

project(genelib LANGUAGES C)

find_program(AWK awk mawk gawk)

file(GLOB HEADERS "${CMAKE_SOURCE_DIR}/src/*.h")
# message("${HEADERS}")

file(GLOB PRESRCS "${CMAKE_SOURCE_DIR}/src/*.p")
# message("${PRESRCS}")

execute_process(COMMAND mkdir -p "${CMAKE_CURRENT_BINARY_DIR}/src")
execute_process(COMMAND mkdir -p "${CMAKE_CURRENT_BINARY_DIR}/include")
foreach (HEADER ${HEADERS})
  execute_process(COMMAND cp "${HEADER}" "${CMAKE_CURRENT_BINARY_DIR}/include")
endforeach()

foreach(PRESRC ${PRESRCS})
  string(REGEX REPLACE ".*/([^/]+)\.p$" "\\1.c" SRC "${PRESRC}")
  set(SRC "${CMAKE_CURRENT_BINARY_DIR}/src/${SRC}")
  list(APPEND SRCS "${SRC}")
  add_custom_command(
    OUTPUT "${SRC}"
    COMMAND "${AWK}" ARGS -f "${CMAKE_SOURCE_DIR}/src/manager.awk" "${PRESRC}" > "${SRC}"
    DEPENDS "${PRESRC}"
  )
endforeach()

#Generate overall header
set(OVERALL_HEADER "${CMAKE_CURRENT_BINARY_DIR}/include/mygene.h")
file(WRITE "${OVERALL_HEADER}")

file(APPEND "${OVERALL_HEADER}" "#ifndef _MYGENE_H\n")
file(APPEND "${OVERALL_HEADER}" "#define _MYGENE_H\n")
file(APPEND "${OVERALL_HEADER}" "#if defined(__cplusplus)\n")
file(APPEND "${OVERALL_HEADER}" "extern \"C\" {\n")
file(APPEND "${OVERALL_HEADER}" "#endif\n")

foreach(HEADER ${HEADERS})
  string(REGEX REPLACE ".*/([^/]+\.h)$" "\\1" HEADER_BASENAME ${HEADER})
  file(APPEND "${OVERALL_HEADER}" "#include \"${HEADER_BASENAME}\"\n")
endforeach()

file(APPEND "${OVERALL_HEADER}" "#if defined(__cplusplus)\n")
file(APPEND "${OVERALL_HEADER}" "}\n")
file(APPEND "${OVERALL_HEADER}" "#endif\n")
file(APPEND "${OVERALL_HEADER}" "#endif\n")

# message(sources "${SRCS}")

add_library(mygene SHARED ${SRCS})
target_include_directories(mygene PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include>
  $<INSTALL_INTERFACE:include>
  )

list(APPEND HEADERS "${OVERALL_HEADER}")

set_target_properties(mygene PROPERTIES PUBLIC_HEADER "${HEADERS}")
# get_target_property(MYGENE_HEADER mygene PUBLIC_HEADER)
# message("${MYGENE_HEADER}")

install(
  TARGETS mygene
  EXPORT mygene-targets
  LIBRARY DESTINATION lib
  PUBLIC_HEADER DESTINATION include/mygene
)

# export(TARGETS mygene NAMESPACE mylib:: FILE mygene.cmake)
# set(CMAKE_EXPORT_PACKAGE_REGISTRY ON)
# export(PACKAGE mygene)
install(
  EXPORT mygene-targets
  NAMESPACE mylib::
  FILE mygene-config.cmake
  DESTINATION lib/cmake/mygene
)