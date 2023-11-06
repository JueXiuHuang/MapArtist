cmake_minimum_required(VERSION 3.14)

project(
  MapArtist
  VERSION 1.0
  LANGUAGES CXX)

file(GLOB_RECURSE SRC_FILES CONFIGURE_DEPENDS
  "${PROJECT_SOURCE_DIR}/src/*.h"
  "${PROJECT_SOURCE_DIR}/src/*.hpp"
  "${PROJECT_SOURCE_DIR}/src/*.cpp"
)
file(GLOB_RECURSE PF_HEADER CONFIGURE_DEPENDS
  "${PROJECT_SOURCE_DIR}/dependency/pathfinding/include/*.h"
  "${PROJECT_SOURCE_DIR}/dependency/pathfinding/include/*.hpp"
)
set(PROTOCOL_VERSION 763)
set(CXX_STANDARD 17)
set(TARGET_NAME mapArtist)

add_executable(${TARGET_NAME} ${SRC_FILES})
target_include_directories(${TARGET_NAME} PUBLIC ${PROJECT_SOURCE_DIR}/Botcraft/include)
target_include_directories(${TARGET_NAME} PUBLIC ${PROJECT_SOURCE_DIR}/dependency/pathfinding/include)
target_precompile_headers(${TARGET_NAME} PUBLIC ${PF_HEADER})
target_link_directories(${TARGET_NAME} PUBLIC ${PROJECT_SOURCE_DIR}/Botcraft/lib)
target_compile_definitions(${TARGET_NAME} PUBLIC "PROTOCOL_VERSION=${PROTOCOL_VERSION}")
target_compile_definitions(${TARGET_NAME} PUBLIC "USE_IMGUI=1")
target_compile_definitions(${TARGET_NAME} PUBLIC "USE_GUI=1")
target_compile_options(${TARGET_NAME} PUBLIC "-g")
target_compile_options(${TARGET_NAME} PUBLIC "-O3")
target_link_libraries(${TARGET_NAME} ws2_32 crypt32)
target_link_libraries(${TARGET_NAME} botcraft protocolCraft)
set_target_properties(${TARGET_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/bin")

