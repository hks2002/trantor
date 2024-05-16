message(STATUS "Updating vcpkg.json")

# Build options
include(cmake/SetBuildOptions.cmake)

# Arguments, override defaults
cmake_policy(SET CMP0054 NEW)

set(ARG_NUM 3) # current index
math(EXPR ARGC_COUNT "${CMAKE_ARGC}")

while(ARG_NUM LESS ARGC_COUNT)
  set(CURRENT_ARG ${CMAKE_ARGV${ARG_NUM}})

  message(STATUS "Processing argument: ${CURRENT_ARG}")

  if("${CURRENT_ARG}" STREQUAL "TRANTOR_USE_SPDLOG")
    set(TRANTOR_USE_SPDLOG ON)
  elseif("${CURRENT_ARG}" STREQUAL "TRANTOR_USE_C-ARES")
    set(TRANTOR_USE_C-ARES ON)
  elseif("${CURRENT_ARG}" STREQUAL "TRANTOR_TLS_OPENSSL")
    set(TRANTOR_TLS_PROVIDER "openssl")
  elseif("${CURRENT_ARG}" STREQUAL "TRANTOR_TLS_BOTAN")
    set(TRANTOR_TLS_PROVIDER "botan3")
  elseif("${CURRENT_ARG}" STREQUAL "BUILD_TESTING")
    set(BUILD_TESTING ON)
  else()
    message(FATAL_ERROR "⛔Unknown argument: ${CURRENT_ARG}")
  endif()

  math(EXPR ARG_NUM "${ARG_NUM}+1") # incrementing current index
endwhile()

if(EXISTS vcpkg.json)
  file(REMOVE vcpkg.json)
endif()

# add start text
set(vcpkg_json
    "{\n\
    \"name\": \"${PROJECT_NAME}\",\n\
    \"version-semver\": \"${TRANTOR_VERSION}\",\n\
    \"description\": \"${PROJECT_DESCRIPTION}\",\n\
    \"homepage\": \"${PROJECT_HOMEPAGE_URL}\",\n\
    \"license\": \"BSD-2-Clause\",\n\
    \"dependencies\": [\n"
)

# add dependencies
if(TRANTOR_USE_SPDLOG)
  list(APPEND vcpkg_deps "spdlog")
endif()
if(TRANTOR_USE_C-ARES)
  list(APPEND vcpkg_deps "c-ares")
endif()
if(TRANTOR_TLS_PROVIDER STREQUAL "auto")
  list(APPEND vcpkg_deps "openssl")
endif()
if(TRANTOR_TLS_PROVIDER STREQUAL "openssl")
  list(APPEND vcpkg_deps "openssl")
endif()
if(TRANTOR_TLS_PROVIDER STREQUAL "botan3")
  list(APPEND vcpkg_deps "botan")
endif()
if(BUILD_TESTING)
  list(APPEND vcpkg_deps "gtest")
endif()

foreach(D ${vcpkg_deps})
  string(APPEND vcpkg_json "      \"${D}\",\n")
endforeach()

# using regex to remove the end ,\
string(
  REGEX REPLACE
        ",\n$"
        ""
        vcpkg_json
        "${vcpkg_json}"
)

# add end text
string(APPEND vcpkg_json "\n\
    ]\n\
}"
)

file(WRITE vcpkg.json ${vcpkg_json})

message(STATUS "Install vcpkg packages")

find_program(VCPKG_EXECUTABLE vcpkg)
if(VCPKG_EXECUTABLE-NOTfOUND)
  message(FATAL_ERROR "⛔vcpkg not found")
endif()

# Install library
execute_process(COMMAND vcpkg x-update-baseline && vcpkg install RESULT_VARIABLE VCPKG_RETURN_CODE)
if(NOT
   VCPKG_RETURN_CODE
   EQUAL
   0
)
  message(FATAL_ERROR "⛔Failed to install vcpkg package")
endif()
