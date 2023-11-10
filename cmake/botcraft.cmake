include(ExternalProject)

set(BOTCRAFT_GIT_REPOSITORY "https://github.com/adepierre/Botcraft")
set(BOTCRAFT_GIT_BRANCH "master")

set(BOTCRAFT_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/dependency/botcraft")
set(BOTCRAFT_BUILD_PATH "${BOTCRAFT_ROOT}/build")
set(BOTCRAFT_INSTALL_PATH "${BOTCRAFT_BUILD_PATH}/install")
set(BOTCRAFT_HEADER_PATH "${BOTCRAFT_INSTALL_PATH}/include")
set(BOTCRAFT_LIB_PATH "${BOTCRAFT_INSTALL_PATH}/lib")
set(BOTCRAFT_BINARY_PATH "${BOTCRAFT_INSTALL_PATH}/bin")

if(NOT EXISTS "${BOTCRAFT_ROOT}/.git")
  message(STATUS "Update submodule")
  execute_process(COMMAND ${GIT_EXECUTABLE} submodule update --init --recursive
                          WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
                          RESULT_VARIABLE GIT_SUBMOD_RESULT)
endif()

file(MAKE_DIRECTORY ${BOTCRAFT_BUILD_PATH})
file(MAKE_DIRECTORY ${BOTCRAFT_INSTALL_PATH})

if(MINGW)
set(BOTCRAFT_CXX_FLAGS "-O3")
elseif(MSVC)
set(BOTCRAFT_CXX_FLAGS "/O2")
endif()

if(MSVC)
  if(NOT CMAKE_GENERATOR_PLATFORM STREQUAL "")
    list(APPEND BOTCRAFT_PLATFORM "-A ${CMAKE_GENERATOR_PLATFORM}")
  endif()
  if(NOT CMAKE_GENERATOR_TOOLSET STREQUAL "")
    list(APPEND BOTCRAFT_PLATFORM "-T ${CMAKE_GENERATOR_TOOLSET}")
  endif()
elseif(MINGW)
  set(BOTCRAFT_PLATFORM "")
endif()
message(STATUS "BOTCRAFT_PLAYFORM: " ${BOTCRAFT_PLATFORM})

ExternalProject_Add(Botcraft
  SOURCE_DIR ${BOTCRAFT_ROOT}
  STEP_TARGETS install
  EXCLUDE_FROM_ALL TRUE
  CMAKE_GENERATOR ${CMAKE_GENERATOR}
  CONFIGURE_COMMAND ${CMAKE_COMMAND} -S ${BOTCRAFT_ROOT} -B ${BOTCRAFT_BUILD_PATH} -G ${CMAKE_GENERATOR} -DBOTCRAFT_USE_OPENGL_GUI=ON -DBOTCRAFT_USE_IMGUI=ON -DCMAKE_CXX_FLAGS=${BOTCRAFT_CXX_FLAGS} -DBOTCRAFT_GAME_VERSION=${MC_VERSION} -DBOTCRAFT_OUTPUT_DIR=${BOTCRAFT_BUILD_PATH} -DBOTCRAFT_COMPRESSION=OFF -DCMAKE_BUILD_TYPE=Release ${BOTCRAFT_PLATFORM}
  BUILD_COMMAND ${CMAKE_COMMAND} --build ${BOTCRAFT_BUILD_PATH} --config Release
  INSTALL_COMMAND ${CMAKE_COMMAND} --install ${BOTCRAFT_BUILD_PATH} --prefix ${BOTCRAFT_INSTALL_PATH} --config Release
)
