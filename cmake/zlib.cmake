include(ExternalProject)

set(ZLIB_GIT_REPOSITORY "https://github.com/madler/zlib")
set(ZLIB_GIT_BRANCH "master")

set(ZLIB_SRC_PATH "${CMAKE_CURRENT_SOURCE_DIR}/dependency/zlib")
set(ZLIB_BUILD_PATH "${CMAKE_CURRENT_BINARY_DIR}/dependency/zlib/build")
set(ZLIB_INSTALL_PATH "${ZLIB_BUILD_PATH}/install")
set(ZLIB_HEADER_PATH "${ZLIB_INSTALL_PATH}/include")
set(ZLIB_LIB_PATH "${ZLIB_INSTALL_PATH}/lib")
set(ZLIB_BINARY_PATH "${ZLIB_INSTALL_PATH}/bin")

if(NOT EXISTS "${ZLIB_SRC_PATH}/.git")
  message(STATUS "Update submodule")
  execute_process(COMMAND ${GIT_EXECUTABLE} submodule update --init --remote --recursive ${ZLIB_SRC_PATH}
                          WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
                          RESULT_VARIABLE GIT_SUBMOD_RESULT)
endif()

ExternalProject_Add(Zlib
  SOURCE_DIR ${ZLIB_SRC_PATH}
  STEP_TARGETS install
  EXCLUDE_FROM_ALL TRUE
  CMAKE_GENERATOR ${CMAKE_GENERATOR}
  CONFIGURE_COMMAND ${CMAKE_COMMAND} -S ${ZLIB_SRC_PATH} -B ${ZLIB_BUILD_PATH} -G ${CMAKE_GENERATOR} -DCMAKE_INSTALL_PREFIX=${ZLIB_INSTALL_PATH} -DCMAKE_BUILD_TYPE=Release ${BUILD_PLATFORM} -DCMAKE_MAKE_PROGRAM=${CMAKE_MAKE_PROGRAM} -DCMAKE_POSITION_INDEPENDENT_CODE=ON
  BUILD_COMMAND ${CMAKE_COMMAND} --build ${ZLIB_BUILD_PATH} --config $<IF:$<CONFIG:Debug>,Debug,Release>
  INSTALL_COMMAND ${CMAKE_COMMAND} --install ${ZLIB_BUILD_PATH} --prefix ${ZLIB_INSTALL_PATH} --config $<IF:$<CONFIG:Debug>,Debug,Release>
)
