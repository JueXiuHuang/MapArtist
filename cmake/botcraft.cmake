include(ExternalProject)

set(BOTCRAFT_GIT_REPOSITORY "https://github.com/adepierre/Botcraft")
set(BOTCRAFT_GIT_BRANCH "master")

set(BOTCRAFT_SRC_PATH "${CMAKE_CURRENT_SOURCE_DIR}/dependency/botcraft")
set(BOTCRAFT_BUILD_PATH "${CMAKE_CURRENT_BINARY_DIR}/dependency/botcraft/build")
set(BOTCRAFT_INSTALL_PATH "${BOTCRAFT_BUILD_PATH}/install")
set(BOTCRAFT_HEADER_PATH "${BOTCRAFT_INSTALL_PATH}/include")
set(BOTCRAFT_LIB_PATH "${BOTCRAFT_INSTALL_PATH}/lib")
set(BOTCRAFT_BINARY_PATH "${BOTCRAFT_INSTALL_PATH}/bin")

if(NOT EXISTS "${BOTCRAFT_SRC_PATH}/.git")
  message(STATUS "Update submodule")
  execute_process(COMMAND ${GIT_EXECUTABLE} submodule update --init --remote --recursive ${BOTCRAFT_SRC_PATH}
                          WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
                          RESULT_VARIABLE GIT_SUBMOD_RESULT)
endif()

ExternalProject_Add(Botcraft
  SOURCE_DIR ${BOTCRAFT_SRC_PATH}
  STEP_TARGETS install
  EXCLUDE_FROM_ALL TRUE
  CMAKE_GENERATOR ${CMAKE_GENERATOR}
  CONFIGURE_COMMAND ${CMAKE_COMMAND} -S ${BOTCRAFT_SRC_PATH} -B ${BOTCRAFT_BUILD_PATH} -G ${CMAKE_GENERATOR} -DBOTCRAFT_USE_OPENGL_GUI=ON -DBOTCRAFT_USE_IMGUI=ON -DCMAKE_CXX_FLAGS=${CMAKE_CXX_FLAGS} -DBOTCRAFT_GAME_VERSION=${MC_VERSION} -DBOTCRAFT_OUTPUT_DIR=${BOTCRAFT_BUILD_PATH} -DBOTCRAFT_COMPRESSION=ON ${BUILD_PLATFORM} -DBOTCRAFT_FORCE_LOCAL_ZLIB=${ZLIB_INSTALL_PATH} -DBOTCRAFT_FORCE_LOCAL_OPENSSL=${OPENSSL_INSTALL_PATH}  -DCMAKE_BUILD_TYPE=Release
  BUILD_COMMAND ${CMAKE_COMMAND} --build ${BOTCRAFT_BUILD_PATH} --config $<IF:$<CONFIG:Debug>,Debug,Release>
  INSTALL_COMMAND ${CMAKE_COMMAND} --install ${BOTCRAFT_BUILD_PATH} --prefix ${BOTCRAFT_INSTALL_PATH} --config $<IF:$<CONFIG:Debug>,Debug,Release>
)

add_dependencies(Botcraft-install Openssl-install Zlib-install)

include_directories(${BOTCRAFT_HEADER_PATH})
link_directories(${BOTCRAFT_LIB_PATH})

if(MINGW)
  list(APPEND BOTCRAFT_DEPEND_DLL ${BOTCRAFT_BINARY_PATH}/libbotcraft.dll)
  list(APPEND BOTCRAFT_DEPEND_DLL ${BOTCRAFT_BINARY_PATH}/libprotocolCraft.dll)
  list(APPEND BOTCRAFT_DEPEND_DLL ${BOTCRAFT_BINARY_PATH}/libstdc++-6.dll)
elseif(MSVC)
  list(APPEND BOTCRAFT_DEPEND_DLL ${BOTCRAFT_BINARY_PATH}/botcraft"$<$<CONFIG:Debug>:_d>".dll)
  list(APPEND BOTCRAFT_DEPEND_DLL ${BOTCRAFT_BINARY_PATH}/protocolCraft"$<$<CONFIG:Debug>:_d>".dll)
endif()
add_custom_command(TARGET Botcraft-install POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
      ${BOTCRAFT_DEPEND_DLL}
      ${MAPARTIST_OUTPUT_DIR})
add_custom_command(TARGET Botcraft-install POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory_if_different
      ${BOTCRAFT_BINARY_PATH}/Assets
      ${MAPARTIST_OUTPUT_DIR}/Assets)
