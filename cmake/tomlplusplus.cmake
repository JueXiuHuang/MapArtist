include(ExternalProject)

set(TOMLPLUSPLUS_GIT_REPOSITORY "https://github.com/marzer/tomlplusplus")
set(TOMLPLUSPLUS_GIT_BRANCH "master")

set(TOMLPLUSPLUS_SRC_PATH "${CMAKE_CURRENT_SOURCE_DIR}/dependency/tomlplusplus")
set(TOMLPLUSPLUS_HEADER_PATH "${TOMLPLUSPLUS_SRC_PATH}/include")

if(NOT EXISTS "${TOMLPLUSPLUS_SRC_PATH}/.git")
  message(STATUS "Update submodule")
  execute_process(COMMAND ${GIT_EXECUTABLE} submodule update --init --remote --recursive ${TOMLPLUSPLUS_SRC_PATH}
                          WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
                          RESULT_VARIABLE GIT_SUBMOD_RESULT)
endif()

file(GLOB_RECURSE TOMLPLUSPLUS_HEADER CONFIGURE_DEPENDS
  "${TOMLPLUSPLUS_HEADER_PATH}/toml++/*.h"
  "${TOMLPLUSPLUS_HEADER_PATH}/toml++/*.hpp"
)

include_directories(${TOMLPLUSPLUS_HEADER_PATH})

