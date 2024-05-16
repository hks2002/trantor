# Get latest verion from conan.io/center
macro(get_conan_latest_build_version lib_name)
  find_program(CURL_EXECUTABLE curl)
  if(CURL_EXECUTABLE-NOTfOUND)
    message(FATAL_ERROR "⛔CURL not found")
  endif()

  # Get latest build info
  execute_process(
    COMMAND curl -v --connect-timeout 2 -L https://conan.io/center/recipes/${lib_name}
    ERROR_VARIABLE CURL_ERROR
    RESULT_VARIABLE CURL_RETURN_CODE
    OUTPUT_VARIABLE html_text
  )
  if(NOT
     CURL_RETURN_CODE
     EQUAL
     0
  )
    message(FATAL_ERROR "⛔Failed to get latest build info from conan.io: ${CURL_ERROR}")
  endif()

  # Get latest build version
  string(REGEX MATCH "\"version\":\"[a-zA-Z]*-*[0-9]+[\\.|-|_][0-9]+[\\.|-|_][0-9]+" latest_build "${html_text}")

  if("${latest_build}" STREQUAL "")
    message(FATAL_ERROR "⛔${lib_name}: Regex Got empty build")
  else()
    string(SUBSTRING "${latest_build}" 11 -1 latest_build_val)
    set(${lib_name}_BUILD_VERSION "${latest_build_val}")
    message(STATUS "${lib_name} latest build: ${latest_build_val}")
  endif()
endmacro()
# ##################################################################################################################################################################################
message(STATUS "Updating conanfile.txt")

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

if(EXISTS conanfile.json)
  file(REMOVE conanfile.json)
endif()

# add start text
set(conanfile_txt "[requires]\n")

# add dependencies
if(TRANTOR_USE_SPDLOG)
  get_conan_latest_build_version(spdlog)
  string(APPEND conanfile_txt "spdlog")
  string(APPEND conanfile_txt "/${spdlog_BUILD_VERSION}\n")
endif()
if(TRANTOR_USE_C-ARES)
  get_conan_latest_build_version(c-ares)
  string(APPEND conanfile_txt "c-ares")
  string(APPEND conanfile_txt "/${c-ares_BUILD_VERSION}\n")
endif()
if(TRANTOR_TLS_PROVIDER STREQUAL "auto")
  get_conan_latest_build_version(openssl)
  string(APPEND conanfile_txt "openssl")
  string(APPEND conanfile_txt "/${openssl_BUILD_VERSION}\n")
endif()
if(TRANTOR_TLS_PROVIDER STREQUAL "openssl")
  get_conan_latest_build_version(openssl)
  string(APPEND conanfile_txt "openssl")
  string(APPEND conanfile_txt "/${openssl_BUILD_VERSION}\n")
endif()
if(TRANTOR_TLS_PROVIDER STREQUAL "botan3")
  get_conan_latest_build_version(botan)
  string(APPEND conanfile_txt "botan")
  string(APPEND conanfile_txt "/${botan_BUILD_VERSION}\n")
endif()
if(BUILD_TESTING)
  get_conan_latest_build_version(gtest)
  string(APPEND conanfile_txt "gtest")
  string(APPEND conanfile_txt "/${gtest_BUILD_VERSION}\n")
endif()

# add end text
string(
  APPEND
  conanfile_txt
  "\

[generators]
CMakeDeps
CMakeToolchain

[options]

[imports]
"
)

file(WRITE conanfile.txt ${conanfile_txt})

message(STATUS "Installing conan packages")

find_program(CONAN_EXECUTABLE conan)
if(CONAN_EXECUTABLE-NOTfOUND)
  message(FATAL_ERROR "⛔conan not found")
endif()

# Install release library
execute_process(COMMAND conan install . --output-folder=build --build=missing --settings=build_type=Release --settings=compiler.runtime=dynamic RESULT_VARIABLE CONAN_RETURN_CODE)
if(NOT
   CONAN_RETURN_CODE
   EQUAL
   0
)
  message(FATAL_ERROR "⛔Failed to install conan package")
endif()

# Install debug library
execute_process(COMMAND conan install . --output-folder=build --build=missing --settings=build_type=Debug --settings=compiler.runtime=dynamic RESULT_VARIABLE CONAN_RETURN_CODE)
if(NOT
   CONAN_RETURN_CODE
   EQUAL
   0
)
  message(FATAL_ERROR "⛔Failed to install conan package")
endif()
