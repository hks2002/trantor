macro(set_openssl)
  list(APPEND TRANTOR_SOURCES trantor/net/tlsprovider/OpenSSLProvider.cc)

  target_link_libraries(${PROJECT_NAME} PRIVATE OpenSSL::SSL OpenSSL::Crypto)
  if(WIN32)
    target_link_libraries(${PROJECT_NAME} PRIVATE Crypt32 Secur32)
  endif()

  target_compile_definitions(${PROJECT_NAME} PRIVATE TRANTOR_TLS_PROVIDER="OpenSSL")
  target_compile_definitions(${PROJECT_NAME} PRIVATE USE_OPENSSL)

  # copy dll/libs for Windows, to solve the test/unittests running 0x000135 error
  copy_files_for_win(${OPENSSL_INCLUDE_DIR}/../bin "dll")
  copy_files_for_win(${OPENSSL_INCLUDE_DIR}/../lib "lib")
endmacro()

# ##################################################################################################################################################################################
message(STATUS "Trantor using SSL library: openssl")

find_package(OpenSSL)
if(NOT OpenSSL_FOUND)
  if(BUILD_MISSING_DEPENDENCIES)
    message(STATUS "⚠️OpenSSL not found, Building it with FetchContent...")
    unset(OpenSSL_FOUND)

    # openssl build need perl
    find_package(Perl REQUIRED)

    if(NOT openssl_DOWNLOAD_URL)
      get_github_latest_release_url("openssl" "openssl")
    endif()
    FetchContent_Declare(openssl URL ${openssl_DOWNLOAD_URL})
    FetchContent_MakeAvailable(openssl)

    # Set OPENSSL_ROOT_DIR, to let find_package could search from the build directory
    set(OPENSSL_ROOT_DIR
        ${CMAKE_INSTALL_PREFIX}/../OpenSSL
        CACHE PATH "Let find_package use the directory to find" FORCE
    )

    if(NOT EXISTS ${OPENSSL_ROOT_DIR}/include/OpenSSL)
      # ⚠️ remove "no-asm" flags to improve openssl performance
      if(CMAKE_CXX_COMPILER_ID MATCHES MSVC)
        execute_process(
          COMMAND ${PERL_EXECUTABLE} ${openssl_SOURCE_DIR}/Configure VC-WIN64A no-stdio no-engine no-apps no-asm --prefix=${OPENSSL_ROOT_DIR}
          WORKING_DIRECTORY ${openssl_BINARY_DIR}
        )
        execute_process(COMMAND nmake WORKING_DIRECTORY ${openssl_BINARY_DIR})
        execute_process(COMMAND nmake install WORKING_DIRECTORY ${openssl_BINARY_DIR})
      else()
        execute_process(COMMAND ${openssl_SOURCE_DIR}/config no-stdio no-engine no-apps no-asm --prefix=${OPENSSL_ROOT_DIR} WORKING_DIRECTORY ${openssl_BINARY_DIR})
        execute_process(COMMAND make -j WORKING_DIRECTORY ${openssl_BINARY_DIR})
        execute_process(COMMAND make install WORKING_DIRECTORY ${openssl_BINARY_DIR})

      endif() # compiler id
    endif()

    # find_again
    find_package(OpenSSL)
    if(OpenSSL_FOUND)
      set_openssl()
    else()
      message(FATAL_ERROR "⛔Not found OpenSSL after build")
    endif()

  else()
    message(FATAL_ERROR "⛔OpenSSL not found, please install it or set TRANTOR_DEPENDENCIES_PROVIDER to source")
  endif()
endif()

set_openssl()
