# Compiler and System settings
message(STATUS "Setting System and Compiler: ${CMAKE_SYSTEM_NAME} - ${CMAKE_CXX_COMPILER_ID} - ${CMAKE_CXX_COMPILER_VERSION}")

# Get lowcase compiler id
macro(get_lowcase_compiler_id)
  if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    set(cmake_cxx_compiler_id "gcc")
  else()
    string(TOLOWER ${CMAKE_CXX_COMPILER_ID} cmake_cxx_ompiler_id)
  endif()
endmacro()
get_lowcase_compiler_id()

# MSVC
if(CMAKE_CXX_COMPILER_ID MATCHES MSVC)
  # Ignore MSVC C4251 and C4275 warning of exporting std objects with no dll export We export class to facilitate maintenance, thus if you compile drogon on windows as a shared
  # library, you will need to use exact same compiler for drogon and your app.
  if(BUILD_SHARED_LIBS)
    target_compile_options(${PROJECT_NAME} PUBLIC /wd4251 /wd4275)
  endif()

  # Tells Visual Studio 2017 (15.7+) and newer to correctly set the value of the standard __cplusplus macro, instead of leaving it to 199711L and settings the effective c++ version
  # in _MSVC_LANG Dropping support for older versions of VS would allow to only rely on __cplusplus
  if(MSVC_VERSION GREATER_EQUAL 1914)
    add_compile_options(/Zc:__cplusplus)
  endif()

endif()

# Clang|GNU not windows
if(CMAKE_CXX_COMPILER_ID MATCHES Clang|GNU)
  if(NOT
     ${CMAKE_SYSTEM_NAME}
     STREQUAL
     "Windows"
  )

    # openssl has some problems with -Werror
    if(BUILD_MISSING_DEPENDENCIES AND TRANTOR_TLS_PROVIDER STREQUAL "openssl")
      target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -Wno-unused-parameter)
    else()
      target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -Werror)
    endif()

  endif()
endif()

# MinGW
if(MINGW)
  target_compile_definitions(${PROJECT_NAME} PUBLIC -D_WIN32_WINNT=0x0601)
endif()

# Haiku
if(${CMAKE_SYSTEM_NAME} STREQUAL "Haiku")
  target_link_libraries(${PROJECT_NAME} PRIVATE network)
endif()

# Socket
if(WIN32)
  target_link_libraries(${PROJECT_NAME} PRIVATE ws2_32 Rpcrt4)
else(NOT ANDROID)
  target_link_libraries(${PROJECT_NAME} PRIVATE pthread $<$<PLATFORM_ID:SunOS>:socket>)
endif()
