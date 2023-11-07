include(ExternalProject)

set(PATHFINDING_GIT_REPOSITORY "https://github.com/ldslds449/PathFinding")
set(PATHFINDING_GIT_BRANCH "main")

set(PATHFINDING_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/dependency/pathfinding")
set(PATHFINDING_HEADER_PATH "${PATHFINDING_ROOT}/include")

if(NOT EXISTS "${PATHFINDING_ROOT}/.git")
  message(STATUS "Update submodule")
  execute_process(COMMAND ${GIT_EXECUTABLE} submodule update --init --recursive
                          WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
                          RESULT_VARIABLE GIT_SUBMOD_RESULT)
endif()

file(GLOB_RECURSE PATHFINDING_HEADER CONFIGURE_DEPENDS
  "${PATHFINDING_HEADER_PATH}/*.h"
  "${PATHFINDING_HEADER_PATH}/*.hpp"
)

