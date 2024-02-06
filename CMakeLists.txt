cmake_minimum_required(VERSION 3.14)

project(
  MapArtist
  VERSION 1.0
  LANGUAGES CXX)

set(MC_VERSION "1.20.1")
set(PROTOCOL_VERSION 763)
set(CXX_STANDARD 17)
set(TARGET_NAME "mapArtist")

find_package(Git QUIET)
if(NOT GIT_FOUND)
  message(FATAL_ERROR "Git not found, please install git first.")
endif()

if(MSVC)
  message(STATUS "Config Type: ${CMAKE_CONFIGURATION_TYPES}")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /O2")
  if(NOT CMAKE_GENERATOR_PLATFORM STREQUAL "")
    list(APPEND BUILD_PLATFORM "-A ${CMAKE_GENERATOR_PLATFORM}")
  endif()
  if(NOT CMAKE_GENERATOR_TOOLSET STREQUAL "")
    list(APPEND BUILD_PLATFORM "-T ${CMAKE_GENERATOR_TOOLSET}")
  endif()
  message(STATUS "BUILD_PLATFORM: " ${BUILD_PLATFORM})
elseif(MINGW)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3")
  set(BUILD_PLATFORM "")
endif()

set(MAPARTIST_OUTPUT_DIR ${PROJECT_SOURCE_DIR}/bin)

include("cmake/pathfinding.cmake")
include("cmake/openssl.cmake")
include("cmake/zlib.cmake")
include("cmake/botcraft.cmake")

if(MSVC)
  include("cmake/dpp.cmake")
endif()

file(GLOB_RECURSE SRC_FILES CONFIGURE_DEPENDS
  "${PROJECT_SOURCE_DIR}/src/*.h"
  "${PROJECT_SOURCE_DIR}/src/*.hpp"
  "${PROJECT_SOURCE_DIR}/src/*.cpp"
)
if(MINGW)
  list(FILTER SRC_FILES EXCLUDE REGEX "Bot\.(hpp|cpp)")
endif()

add_executable(${TARGET_NAME} ${SRC_FILES})

if(MINGW)
  set(MAPARTIST_CXX_FLAGS "-std=c++17")
  set(MAPARTIST_CXX_FLAGS $<$<CONFIG:Debug>:-g>)
elseif(MSVC)
  set(MAPARTIST_CXX_FLAGS "/std:c++17" "/utf-8")
endif()

target_compile_options(${TARGET_NAME} PUBLIC ${MAPARTIST_CXX_FLAGS})

# botcraft
add_dependencies(${TARGET_NAME} Botcraft-install)
target_compile_definitions(${TARGET_NAME} PUBLIC "PROTOCOL_VERSION=${PROTOCOL_VERSION}")
target_compile_definitions(${TARGET_NAME} PUBLIC "USE_IMGUI=1")
target_compile_definitions(${TARGET_NAME} PUBLIC "USE_GUI=1")
if(MINGW)
  target_link_libraries(${TARGET_NAME} ws2_32 botcraft protocolCraft)
elseif(MSVC)
  message(STATUS "Asan: ${Asan}")
  target_compile_options(${TARGET_NAME} PUBLIC $<$<AND:$<CONFIG:Debug>,$<BOOL:${Asan}>>:/fsanitize=address>)
  target_link_libraries(${TARGET_NAME} ws2_32 botcraft$<$<CONFIG:Debug>:_d> protocolCraft$<$<CONFIG:Debug>:_d>)
endif()

# pathfinding
target_include_directories(${TARGET_NAME} PUBLIC ${PATHFINDING_HEADER_PATH})
target_precompile_headers(${TARGET_NAME} PUBLIC ${PATHFINDING_HEADER})

# dpp
if(MSVC)
  add_dependencies(${TARGET_NAME} Dpp-install)
  target_link_libraries(${TARGET_NAME} dpp)
endif()

get_filename_component(COMPILER_FOLDER ${CMAKE_CXX_COMPILER} DIRECTORY)
if(MINGW)
  list(APPEND MAPARTIST_DEPEND_DLL ${COMPILER_FOLDER}/libstdc++-6.dll)
  list(APPEND MAPARTIST_DEPEND_DLL ${COMPILER_FOLDER}/libgcc_s_seh-1.dll)
  list(APPEND MAPARTIST_DEPEND_DLL ${COMPILER_FOLDER}/libwinpthread-1.dll)
  add_custom_command(TARGET ${TARGET_NAME} POST_BUILD
          COMMAND ${CMAKE_COMMAND} -E copy_if_different
          ${MAPARTIST_DEPEND_DLL}
          $<TARGET_FILE_DIR:${TARGET_NAME}>)
elseif(MSVC)
  list(APPEND MAPARTIST_DEPEND_DLL ${COMPILER_FOLDER}/clang_rt.asan_dynamic-x86_64.dll)
  if(${Asan} STREQUAL "1")
    add_custom_command(TARGET ${TARGET_NAME} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
            ${MAPARTIST_DEPEND_DLL}
            $<TARGET_FILE_DIR:${TARGET_NAME}>)
  endif()
endif()

if(MSVC)
  set_target_properties(${TARGET_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${MAPARTIST_OUTPUT_DIR})
  set_target_properties(${TARGET_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY_DEBUG ${MAPARTIST_OUTPUT_DIR})
  set_target_properties(${TARGET_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY_RELEASE ${MAPARTIST_OUTPUT_DIR})
elseif(MINGW)
  set_target_properties(${TARGET_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${MAPARTIST_OUTPUT_DIR})
endif()

