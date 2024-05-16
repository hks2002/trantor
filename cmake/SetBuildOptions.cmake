option(BUILD_SHARED_LIBS "Build ${PROJECT_NAME} as a shared lib" ON)
option(BUILD_TESTING "Build tests" ON)
option(BUILD_DOC "Build Doxygen documentation" OFF)
option(BUILD_MISSING_DEPENDENCIES "Fetch and build missing dependencies, CMake version >=3.11 required" ON)
option(TRANTOR_USE_SPDLOG "Allow using the spdlog logging library" ON)
option(TRANTOR_USE_C-ARES "Allow using C-ARES" ON)
option(TRANTOR_TLS_PROVIDER "TLS(Transport Layer Security) provider for ${PROJECT_NAME}. Valid options are 'none', 'openssl', 'botan-3', 'auto'." "none")
if(NOT TRANTOR_TLS_PROVIDER)
  set(TRANTOR_TLS_PROVIDER
      "none"
      CACHE STRING "" FORCE
  )
endif()
