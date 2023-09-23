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
set(PROTOCOL_VERSION 763)
set(CXX_STANDARD 17)
set(TARGET_NAME mapArtist)

add_executable(${TARGET_NAME} ${SRC_FILES})
target_include_directories(${TARGET_NAME} PUBLIC ${PROJECT_SOURCE_DIR}/Botcraft/include)
target_link_directories(${TARGET_NAME} PUBLIC ${PROJECT_SOURCE_DIR}/Botcraft/lib)
target_compile_definitions(${TARGET_NAME} PUBLIC "PROTOCOL_VERSION=${PROTOCOL_VERSION}")
target_compile_definitions(${TARGET_NAME} PUBLIC "USE_IMGUI=1")
target_compile_definitions(${TARGET_NAME} PUBLIC "USE_GUI=1")
target_compile_options(${TARGET_NAME} PUBLIC "-g")
target_link_libraries(${TARGET_NAME} ws2_32 crypt32)
target_link_libraries(${TARGET_NAME} botcraft protocolCraft)
set_target_properties(${TARGET_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/bin")

