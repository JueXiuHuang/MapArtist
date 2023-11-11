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
# include("cmake/opus.cmake")
# include("cmake/sodium.cmake")
# include("cmake/openssl.cmake")
# include("cmake/jsonifier.cmake")
# include("cmake/zlib.cmake")
# include("cmake/discordCoreAPI.cmake")
# include("cmake/dpp.cmake")

file(GLOB_RECURSE SRC_FILES CONFIGURE_DEPENDS
  "${PROJECT_SOURCE_DIR}/src/*.h"
  "${PROJECT_SOURCE_DIR}/src/*.hpp"
  "${PROJECT_SOURCE_DIR}/src/*.cpp"
)

add_executable(${TARGET_NAME} ${SRC_FILES})

if(MINGW)
  set(MAPARTIST_CXX_FLAGS "-O3" "-std=c++17")
  set(MAPARTIST_DEPEND_DLL 
    "${BOTCRAFT_BINARY_PATH}/libbotcraft.dll"
    "${BOTCRAFT_BINARY_PATH}/libprotocolCraft.dll"
    "${BOTCRAFT_BINARY_PATH}/libstdc++-6.dll")
elseif(MSVC)
  set(MAPARTIST_CXX_FLAGS "/std:c++17" "/utf-8")
  set(MAPARTIST_DEPEND_DLL 
    "${BOTCRAFT_BINARY_PATH}/botcraft.dll"
    "${BOTCRAFT_BINARY_PATH}/protocolCraft.dll")
endif()

target_compile_options(${TARGET_NAME} PUBLIC ${MAPARTIST_CXX_FLAGS})

add_dependencies(${TARGET_NAME} Botcraft-install)
target_include_directories(${TARGET_NAME} PUBLIC ${BOTCRAFT_HEADER_PATH})
target_link_directories(${TARGET_NAME} PUBLIC ${BOTCRAFT_LIB_PATH})
target_compile_definitions(${TARGET_NAME} PUBLIC "PROTOCOL_VERSION=${PROTOCOL_VERSION}")
target_compile_definitions(${TARGET_NAME} PUBLIC "USE_IMGUI=1")
target_compile_definitions(${TARGET_NAME} PUBLIC "USE_GUI=1")
target_link_libraries(${TARGET_NAME} ws2_32)
target_link_libraries(${TARGET_NAME} botcraft protocolCraft)

target_include_directories(${TARGET_NAME} PUBLIC ${PATHFINDING_HEADER_PATH})
target_precompile_headers(${TARGET_NAME} PUBLIC ${PATHFINDING_HEADER})

# add_dependencies(${TARGET_NAME} Opus-install)
# add_dependencies(${TARGET_NAME} Sodium-install)
# add_dependencies(${TARGET_NAME} Openssl-install)
# add_dependencies(${TARGET_NAME} Jsonifier-install)
# add_dependencies(${TARGET_NAME} Zlib-install)
# add_dependencies(${TARGET_NAME} DiscordCoreAPI-install)
# add_dependencies(${TARGET_NAME} Dpp-install)

if(MSVC)
  set_target_properties(${TARGET_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/bin)
  set_target_properties(${TARGET_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY_DEBUG ${PROJECT_SOURCE_DIR}/bin)
  set_target_properties(${TARGET_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY_RELEASE ${PROJECT_SOURCE_DIR}/bin)
elseif(MINGW)
  set_target_properties(${TARGET_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/bin)
endif()


add_custom_command(TARGET ${TARGET_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
      ${MAPARTIST_DEPEND_DLL}
      $<TARGET_FILE_DIR:${TARGET_NAME}>)

add_custom_command(TARGET ${TARGET_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
      "${BOTCRAFT_BINARY_PATH}/Assets"
      "$<TARGET_FILE_DIR:${TARGET_NAME}>/Assets")
