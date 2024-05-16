macro(get_compiler_id)
  # Get lowercase compiler id
  string(TOLOWER "${CMAKE_CXX_COMPILER_ID}" cmake_cxx_compiler_id)
  if(cmake_cxx_compiler_id STREQUAL "gnu")
    set(cmake_cxx_compiler_id "gcc")
  endif()
endmacro()

macro(get_download_url org_name lib_name)
  if(${lib_name}_VER)
    set(${lib_name}_DOWNLOAD_URL "https://github.com/${org_name}/${lib_name}/archive/refs/tags/${lib_name_VER}.zip")
  else()
    execute_process(
      COMMAND curl -sL https://api.github.com/repos/${org_name}/${lib_name}/releases/latest OUTPUT_VARIABLE latest_json
    )
    string(
      REGEX MATCH
            "\"tag_name\": \"[a-zA-Z]*-*[0-9]+[\\.|-|_][0-9]+[\\.|-|_][0-9]+"
            latest_tag
            "${latest_json}"
    )

    if(${latest_tag} STREQUAL "")
      message(FATAL_ERROR "REGEX Got empty tag")
    else()
      message(STATUS "Latest tag: ${latest_tag}")
      message(STATUS "Passing ${lib_name}_VER to change version")
    endif()

    string(
      SUBSTRING "${latest_tag}"
                13
                -1
                latest_tag_val
    )

    set(${lib_name}_DOWNLOAD_URL "https://github.com/${org_name}/${lib_name}/archive/refs/tags/${latest_tag_val}.zip")
  endif()
endmacro()

# if set toolchain file, don't build, and add include path and link path
if(CMAKE_TOOLCHAIN_FILE)
  set(BUILD_DEPENDENCIES
      OFF
      CACHE BOOL "Build Dependencies" FORCE
  )
  # toolchain file added include path and link path, use it
  target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_INCLUDE_PATH})
  target_link_directories(${PROJECT_NAME} PRIVATE ${CMAKE_LIBRARY_PATH})
endif()

if(BUILD_DEPENDENCIES)
  message(STATUS "Building Dependencies")

  # CMake >= 3.14
  if(${CMAKE_VERSION} VERSION_LESS "3.14")
    message(FATAL_ERROR "CMake >= 3.14 required")
  endif()

  include(FetchContent)
  # show progress
  set(FETCHCONTENT_QUIET OFF)
  set(FETCHCONTENT_UPDATES_DISCONNECTED ON)

  # disable CMP0135 warning
  if(${CMAKE_VERSION} VERSION_GREATER_EQUAL "3.24")
    cmake_policy(SET CMP0135 NEW)
  endif()

  # 💡Using URL is faster than git clone

  # GTest/GoogleTest
  if(BUILD_TESTING)
    #[[
    # Can't use name ⚠️GTest/gtest⚠️ in FetchContent_Declare, see:
    # CMake FetchContent not working as expected (https://github.com/google/googletest/issues/2457)
    # FetchContent_* does not work with GTest as dependency name on Windows (https://gitlab.kitware.com/cmake/cmake/-/issues/25294)
    # The name gtest reserved:
    # googletest_SOURCE_DIR: build/_deps/googletest-src
    # gtest_SOURCE_DIR: build/_deps/googletest-src/googletest
    #]]
    get_download_url("google" "googletest")
    FetchContent_Declare(googletest URL ${googletest_DOWNLOAD_URL})
    # For Windows: Prevent overriding the parent project's compiler/linker ⚠️MT/MD⚠️ settings
    set(gtest_force_shared_crt
        ON
        CACHE BOOL "" FORCE
    )
    FetchContent_MakeAvailable(googletest)

    # Set build output path, this is mainly for Windows, to solve the test/unittests running 0x000135 problems
    set_standard_build_output(gtest)
    set_standard_build_output(gtest_main)
    set_standard_build_output(gmock)
    set_standard_build_output(gmock_main)
  endif()

  # spdlog
  if(TRANTOR_USE_SPDLOG)
    get_download_url("gabime" "spdlog")
    get_download_url("fmtlib" "fmt")
    FetchContent_Declare(spdlog URL ${spdlog_DOWNLOAD_URL})
    FetchContent_Declare(fmt URL ${fmt_DOWNLOAD_URL})
    FetchContent_MakeAvailable(fmt spdlog)

    # Set build output path, this is mainly for Windows, to solve the test/unittests running 0x000135 problems
    set_standard_build_output(fmt)
    set_standard_build_output(spdlog)
  endif()

  # c-ares
  if(TRANTOR_USE_C-ARES)
    get_download_url("c-ares" "c-ares")
    FetchContent_Declare(c-ares URL ${c-ares_DOWNLOAD_URL})
    FetchContent_MakeAvailable(c-ares)

    # Set build output path, this is mainly for Windows, to solve the test/unittests running 0x000135 problems
    set_standard_build_output(c-ares)
  endif()

  # OpenSSL
  if(TRANTOR_TLS_PROVIDER STREQUAL "openssl")
    # openssl build need perl
    find_package(Perl REQUIRED)

    get_download_url("openssl" "openssl")
    FetchContent_Declare(openssl URL ${openssl_DOWNLOAD_URL})
    FetchContent_MakeAvailable(OpenSSL)

    # Set OPENSSL_ROOT_DIR, to let find_package could search from the build directory
    set(OPENSSL_ROOT_DIR
        ${FETCHCONTENT_BASE_DIR}/openssl
        CACHE PATH "Let find_package use the directory to find" FORCE
    )

    if(NOT EXISTS ${OPENSSL_ROOT_DIR}/include/openssl)
      # ⚠️ remove "no-asm" flags to improve openssl performance
      if(CMAKE_CXX_COMPILER_ID MATCHES MSVC)
        execute_process(
          COMMAND ${PERL_EXECUTABLE} ${openssl_SOURCE_DIR}/Configure VC-WIN64A no-asm no-tests
                  --prefix=${OPENSSL_ROOT_DIR} WORKING_DIRECTORY ${openssl_BINARY_DIR}
        )
        execute_process(COMMAND nmake WORKING_DIRECTORY ${openssl_BINARY_DIR})
        execute_process(COMMAND nmake install WORKING_DIRECTORY ${openssl_BINARY_DIR})
      else()
        execute_process(
          COMMAND ${openssl_SOURCE_DIR}/config no-asm no-tests --prefix=${OPENSSL_ROOT_DIR}
          WORKING_DIRECTORY ${openssl_BINARY_DIR}
        )
        execute_process(COMMAND make -j WORKING_DIRECTORY ${openssl_BINARY_DIR})
        execute_process(COMMAND make install WORKING_DIRECTORY ${openssl_BINARY_DIR})
      endif() # compiler id
    endif()

    target_include_directories(${PROJECT_NAME} PRIVATE ${OPENSSL_ROOT_DIR}/include)
    # copy dlls, this is mainly for Windows, to solve the test/unittests running 0x000135 problems
    copy_dlls_to_standard_build_output(${OPENSSL_ROOT_DIR}/bin)

  endif()

  # Botan-3
  #[[
    # ⚠️⚠️⚠️Botan3 is now a C++20 codebase; compiler requirements have been increased to GCC 11, Clang 14, or MSVC 2022. (GH #2455 #3086)
    # support ninja build after botan 3.2
    #]]
  if(TRANTOR_TLS_PROVIDER STREQUAL "botan-3")
    # botan build need python
    find_package(PythonInterp REQUIRED)

    get_download_url("randombit" "botan")
    FetchContent_Declare(botan URL ${botan_DOWNLOAD_URL})
    FetchContent_MakeAvailable(Botan)

    set(BOTAN_ROOT_DIR
        ${FETCHCONTENT_BASE_DIR}/botan-3
        CACHE PATH "Let find_package use the directory to find" FORCE
    )

    if(NOT EXISTS ${BOTAN_ROOT_DIR}/include/botan-3/botan)
      # Get lowercase compiler id
      get_compiler_id()

      if(CMAKE_CXX_COMPILER_ID MATCHES MSVC)
        # ⚠️botan-3 support ninja build
        execute_process(
          COMMAND ${PYTHON_EXECUTABLE} ${botan_SOURCE_DIR}/configure.py --prefix=${BOTAN_ROOT_DIR}
                  --cc=${cmake_cxx_compiler_id} --build-tool=ninja --with-pkg-config --build-target=shared
          WORKING_DIRECTORY ${botan_BINARY_DIR}
        )
        execute_process(COMMAND ninja -j WORKING_DIRECTORY ${botan_BINARY_DIR})
        execute_process(COMMAND ninja install WORKING_DIRECTORY ${botan_BINARY_DIR})
      else()
        execute_process(
          COMMAND ${PYTHON_EXECUTABLE} ${botan_SOURCE_DIR}/configure.py --prefix=${BOTAN_ROOT_DIR}
                  --cc=${cmake_cxx_compiler_id} --with-pkg-config --build-target=shared
          WORKING_DIRECTORY ${botan_BINARY_DIR}
        )
        execute_process(COMMAND make -j WORKING_DIRECTORY ${botan_BINARY_DIR})
        execute_process(COMMAND make install WORKING_DIRECTORY ${botan_BINARY_DIR})
      endif() # compiler id
    endif()

    target_include_directories(${PROJECT_NAME} PRIVATE ${BOTAN_ROOT_DIR}/include/botan-3)
    # copy dlls, this is mainly for Windows, to solve the test/unittests running 0x000135 problems
    copy_dlls_to_standard_build_output(${BOTAN_ROOT_DIR}/bin)

  endif()

endif()
