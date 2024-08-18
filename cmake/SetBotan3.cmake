macro(set_botan3)

  target_compile_definitions(${PROJECT_NAME} PRIVATE TRANTOR_TLS_PROVIDER="Botan")
  target_compile_definitions(${PROJECT_NAME} PRIVATE USE_BOTAN)

  target_include_directories(${PROJECT_NAME} PRIVATE ${BOTAN_ROOT_DIR}/include/botan-3)

  # conan target is different, always lower case
  if(TARGET botan::botan)
    target_link_libraries(${PROJECT_NAME} PRIVATE botan::botan)
    target_compile_features(botan::botan INTERFACE cxx_std_20)
  else()
    target_link_libraries(${PROJECT_NAME} PRIVATE Botan::Botan)
    target_compile_features(Botan::Botan INTERFACE cxx_std_20)
  endif()

  list(APPEND TRANTOR_SOURCES trantor/net/tlsprovider/BotanTLSProvider.cc)

  # copy dll/libs for Windows, to solve the test/unittests running 0x000135 error
  copy_files_for_win(${Botan_INCLUDE_DIRS}/../../bin "dll")
  copy_files_for_win(${Botan_INCLUDE_DIRS}/../../lib "lib")
endmacro()

# ##################################################################################################################################################################################
message(STATUS "Trantor using SSL library: botan")

find_package(Botan 3)
if(NOT Botan_FOUND)
  if(BUILD_MISSING_DEPENDENCIES)
    message(STATUS "⚠️Botan not found, Building it with FetchContent...")
    unset(Botan_FOUND)

    # botan build need python
    find_package(PythonInterp REQUIRED)

    if(NOT botan_DOWNLOAD_URL)
      get_github_latest_release_url("randombit" "botan")
    endif()
    FetchContent_Declare(Botan URL ${botan_DOWNLOAD_URL})
    FetchContent_MakeAvailable(Botan)

  else()
    message(FATAL_ERROR "⛔Botan not found, please install it or set BUILD_MISSING_DEPENDENCIES to ON")
  endif()
endif()

set_botan3()
