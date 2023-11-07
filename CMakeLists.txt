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

include("cmake/botcraft.cmake")
include("cmake/pathfinding.cmake")

file(GLOB_RECURSE SRC_FILES CONFIGURE_DEPENDS
  "${PROJECT_SOURCE_DIR}/src/*.h"
  "${PROJECT_SOURCE_DIR}/src/*.hpp"
  "${PROJECT_SOURCE_DIR}/src/*.cpp"
)

add_executable(${TARGET_NAME} ${SRC_FILES})

target_compile_options(${TARGET_NAME} PUBLIC "-g")
target_compile_options(${TARGET_NAME} PUBLIC "-O3")

add_dependencies(${TARGET_NAME} Botcraft-install)
target_include_directories(${TARGET_NAME} PUBLIC ${BOTCRAFT_HEADER_PATH})
target_link_directories(${TARGET_NAME} PUBLIC ${BOTCRAFT_LIB_PATH})
target_compile_definitions(${TARGET_NAME} PUBLIC "PROTOCOL_VERSION=${PROTOCOL_VERSION}")
target_compile_definitions(${TARGET_NAME} PUBLIC "USE_IMGUI=1")
target_compile_definitions(${TARGET_NAME} PUBLIC "USE_GUI=1")
target_link_libraries(${TARGET_NAME} ws2_32 crypt32)
target_link_libraries(${TARGET_NAME} botcraft protocolCraft)

target_include_directories(${TARGET_NAME} PUBLIC ${PATHFINDING_HEADER_PATH})
target_precompile_headers(${TARGET_NAME} PUBLIC ${PATHFINDING_HEADER})

set_target_properties(${TARGET_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/bin")

add_custom_command(TARGET ${TARGET_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
      "${BOTCRAFT_BINARY_PATH}/libbotcraft.dll"
      "${BOTCRAFT_BINARY_PATH}/libprotocolCraft.dll"
      "${BOTCRAFT_BINARY_PATH}/libstdc++-6.dll"
      $<TARGET_FILE_DIR:${TARGET_NAME}>)

add_custom_command(TARGET ${TARGET_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
      "${BOTCRAFT_BINARY_PATH}/Assets"
      "$<TARGET_FILE_DIR:${TARGET_NAME}>/Assets")
