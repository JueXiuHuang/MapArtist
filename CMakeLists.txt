cmake_minimum_required(VERSION 3.14)

project(
  MapArtist
  VERSION 1.0
  LANGUAGES CXX)

set(MC_VERSION "1.20.1")
set(PROTOCOL_VERSION 763)
set(CXX_STANDARD 17)
set(TARGET_NAME "mapArtist")

# check git
find_package(Git QUIET)
if(NOT GIT_FOUND)
  message(FATAL_ERROR "Git not found, please install git first.")
endif()

# check compiler
if(MINGW)
message(FATAL_ERROR "We do not support MinGW compiler.")
endif()

message(STATUS "Config Type: ${CMAKE_CONFIGURATION_TYPES}")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /O2")
if(NOT CMAKE_GENERATOR_PLATFORM STREQUAL "")
  list(APPEND BUILD_PLATFORM "-A ${CMAKE_GENERATOR_PLATFORM}")
endif()
if(NOT CMAKE_GENERATOR_TOOLSET STREQUAL "")
  list(APPEND BUILD_PLATFORM "-T ${CMAKE_GENERATOR_TOOLSET}")
endif()
message(STATUS "BUILD_PLATFORM: " ${BUILD_PLATFORM})

set(MAPARTIST_OUTPUT_DIR ${PROJECT_SOURCE_DIR}/bin)

include("cmake/pathfinding.cmake")
include("cmake/openssl.cmake")
include("cmake/zlib.cmake")
include("cmake/botcraft.cmake")
include("cmake/dpp.cmake")
include("cmake/tomlplusplus.cmake")

file(GLOB_RECURSE SRC_FILES CONFIGURE_DEPENDS
  "${PROJECT_SOURCE_DIR}/src/*.h"
  "${PROJECT_SOURCE_DIR}/src/*.hpp"
  "${PROJECT_SOURCE_DIR}/src/*.cpp"
)

add_executable(${TARGET_NAME} ${SRC_FILES})

# flags
set(MAPARTIST_CXX_FLAGS "/std:c++17" "/utf-8")
target_compile_options(${TARGET_NAME} PUBLIC ${MAPARTIST_CXX_FLAGS})

if(NOT Asan)
  set(Asan 0)
endif()
message(STATUS "Asan: ${Asan}")

# botcraft
add_dependencies(${TARGET_NAME} Botcraft-install)
target_compile_definitions(${TARGET_NAME} PUBLIC "PROTOCOL_VERSION=${PROTOCOL_VERSION}")
target_compile_definitions(${TARGET_NAME} PUBLIC "USE_IMGUI=1")
target_compile_definitions(${TARGET_NAME} PUBLIC "USE_GUI=1")
target_compile_options(${TARGET_NAME} PUBLIC $<$<AND:$<CONFIG:Debug>,$<BOOL:${Asan}>>:/fsanitize=address>)
target_link_libraries(${TARGET_NAME} ws2_32 botcraft$<$<CONFIG:Debug>:_d> protocolCraft$<$<CONFIG:Debug>:_d>)

# pathfinding
target_include_directories(${TARGET_NAME} PUBLIC ${PATHFINDING_HEADER_PATH})
target_precompile_headers(${TARGET_NAME} PUBLIC ${PATHFINDING_HEADER})

# dpp
add_dependencies(${TARGET_NAME} Dpp-install)
target_link_libraries(${TARGET_NAME} dpp)

# tomlplusplus
target_include_directories(${TARGET_NAME} PUBLIC ${TOMLPLUSPLUS_HEADER_PATH})

# copy necessary dll when using ASAN
get_filename_component(COMPILER_FOLDER ${CMAKE_CXX_COMPILER} DIRECTORY)
list(APPEND MAPARTIST_DEPEND_DLL ${COMPILER_FOLDER}/clang_rt.asan_dynamic-x86_64.dll)
if(${Asan} STREQUAL "1")
  add_custom_command(TARGET ${TARGET_NAME} POST_BUILD
          COMMAND ${CMAKE_COMMAND} -E copy_if_different
          ${MAPARTIST_DEPEND_DLL}
          $<TARGET_FILE_DIR:${TARGET_NAME}>)
endif()

set_target_properties(${TARGET_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${MAPARTIST_OUTPUT_DIR})
set_target_properties(${TARGET_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY_DEBUG ${MAPARTIST_OUTPUT_DIR})
set_target_properties(${TARGET_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY_RELEASE ${MAPARTIST_OUTPUT_DIR})
