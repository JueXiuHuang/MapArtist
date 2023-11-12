include(ExternalProject)

set(PATHFINDING_GIT_REPOSITORY "https://github.com/ldslds449/PathFinding")
set(PATHFINDING_GIT_BRANCH "main")

set(PATHFINDING_SRC_PATH "${CMAKE_CURRENT_SOURCE_DIR}/dependency/pathfinding")
set(PATHFINDING_HEADER_PATH "${PATHFINDING_SRC_PATH}/include")

if(NOT EXISTS "${PATHFINDING_SRC_PATH}/.git")
  message(STATUS "Update submodule")
  execute_process(COMMAND ${GIT_EXECUTABLE} submodule update --init --remote --recursive ${PATHFINDING_SRC_PATH}
                          WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
                          RESULT_VARIABLE GIT_SUBMOD_RESULT)
endif()

file(GLOB_RECURSE PATHFINDING_HEADER CONFIGURE_DEPENDS
  "${PATHFINDING_HEADER_PATH}/*.h"
  "${PATHFINDING_HEADER_PATH}/*.hpp"
)

include_directories(${PATHFINDING_HEADER_PATH})

