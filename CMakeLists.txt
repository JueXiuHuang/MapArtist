cmake_minimum_required(VERSION 3.26)

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

# options
option(ASAN "Whether to use address sanitizer" OFF)
option(REBUILD "Whether to always rebuild the dependencies" OFF)

message(STATUS "Asan: ${ASAN}")
message(STATUS "Rebuild: ${REBUILD}")

message(STATUS "Config Type: ${CMAKE_CONFIGURATION_TYPES}")
if(MSVC)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /O2")
elseif(GCC)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3")
endif()
if(NOT CMAKE_GENERATOR_PLATFORM STREQUAL "")
  list(APPEND BUILD_PLATFORM "-A ${CMAKE_GENERATOR_PLATFORM}")
endif()
if(NOT CMAKE_GENERATOR_TOOLSET STREQUAL "")
  list(APPEND BUILD_PLATFORM "-T ${CMAKE_GENERATOR_TOOLSET}")
endif()
message(STATUS "BUILD_PLATFORM: " ${BUILD_PLATFORM})

set(MAPARTIST_OUTPUT_DIR ${PROJECT_SOURCE_DIR}/bin)

include("cmake/pathfinding.cmake")
include("cmake/botcraft.cmake")
include("cmake/dpp.cmake")
include("cmake/tomlplusplus.cmake")

file(GLOB_RECURSE SRC_FILES CONFIGURE_DEPENDS
  "${PROJECT_SOURCE_DIR}/src/*.h"
  "${PROJECT_SOURCE_DIR}/src/*.hpp"
  "${PROJECT_SOURCE_DIR}/src/*.cpp"
)

# icon
if(WIN32)
  enable_language("RC")
  set(WIN32_RESOURCES ${CMAKE_CURRENT_SOURCE_DIR}/icon/icon.rc)
endif()

add_executable(${TARGET_NAME} ${SRC_FILES} ${WIN32_RESOURCES})

# flags
target_compile_features(${TARGET_NAME} PRIVATE cxx_std_17)
target_compile_options(${TARGET_NAME} PUBLIC ${MAPARTIST_CXX_FLAGS})
target_compile_options(${TARGET_NAME} PUBLIC "$<$<CXX_COMPILER_ID:MSVC>:/utf-8>")

# botcraft
add_dependencies(${TARGET_NAME} Botcraft-install)
target_compile_definitions(${TARGET_NAME} PUBLIC "PROTOCOL_VERSION=${PROTOCOL_VERSION}")
target_compile_options(${TARGET_NAME} PUBLIC $<$<AND:$<CONFIG:Debug>,$<BOOL:${ASAN}>>:/fsanitize=address>)
target_link_libraries(${TARGET_NAME} 
  $<$<PLATFORM_ID:Windows>:ws2_32>
  $<$<PLATFORM_ID:Linux>:pthread>
  botcraft$<$<CONFIG:Debug>:_d>
  protocolCraft$<$<CONFIG:Debug>:_d>)

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
if(${ASAN} STREQUAL "1")
  add_custom_command(TARGET ${TARGET_NAME} POST_BUILD
          COMMAND ${CMAKE_COMMAND} -E copy_if_different
          ${MAPARTIST_DEPEND_DLL}
          $<TARGET_FILE_DIR:${TARGET_NAME}>)
endif()

set_target_properties(${TARGET_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${MAPARTIST_OUTPUT_DIR})
set_target_properties(${TARGET_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY_DEBUG ${MAPARTIST_OUTPUT_DIR})
set_target_properties(${TARGET_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY_RELEASE ${MAPARTIST_OUTPUT_DIR})
