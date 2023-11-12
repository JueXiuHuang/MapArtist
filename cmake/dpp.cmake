include(ExternalProject)

set(DPP_GIT_REPOSITORY "https://github.com/brainboxdotcc/DPP")
set(DPP_GIT_BRANCH "master")

set(DPP_SRC_PATH "${CMAKE_CURRENT_SOURCE_DIR}/dependency/dpp")
set(DPP_BUILD_PATH "${CMAKE_CURRENT_BINARY_DIR}/dependency/dpp/build")
set(DPP_INSTALL_PATH "${DPP_BUILD_PATH}/install")
set(DPP_HEADER_PATH "${DPP_INSTALL_PATH}/include")
set(DPP_LIB_PATH "${DPP_INSTALL_PATH}/lib")
set(DPP_BINARY_PATH "${DPP_INSTALL_PATH}/bin")

if(NOT EXISTS "${DPP_SRC_PATH}/.git")
  message(STATUS "Update submodule")
  execute_process(COMMAND ${GIT_EXECUTABLE} submodule update --init --remote --recursive
                          WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
                          RESULT_VARIABLE GIT_SUBMOD_RESULT)
endif()


ExternalProject_Add(Dpp
  SOURCE_DIR ${DPP_SRC_PATH}
  STEP_TARGETS install
  EXCLUDE_FROM_ALL TRUE
  CMAKE_GENERATOR ${CMAKE_GENERATOR}
  CONFIGURE_COMMAND ${CMAKE_COMMAND} -S ${DPP_SRC_PATH} -B ${DPP_BUILD_PATH} -G ${CMAKE_GENERATOR} -DDPP_NO_VCPKG=ON ${BUILD_PLATFORM} -DCMAKE_CXX_FLAGS=${CMAKE_CXX_FLAGS} -DDPP_BUILD_TEST=OFF -DCMAKE_BUILD_TYPE=Release
  BUILD_COMMAND ${CMAKE_COMMAND} --build ${DPP_BUILD_PATH} --config Release
  INSTALL_COMMAND ${CMAKE_COMMAND} --install ${DPP_BUILD_PATH} --prefix ${DPP_INSTALL_PATH}
)
