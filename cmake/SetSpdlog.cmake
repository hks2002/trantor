message(STATUS "Setting for spdlog: ${TRANTOR_USE_SPDLOG}")

if(TRANTOR_USE_SPDLOG)
  find_package(spdlog)

  if(NOT spdlog_FOUND)
    if(BUILD_MISSING_DEPENDENCIES)
      message(STATUS "⚠️spdlog not found, Building it with FetchContent...")
      unset(spdlog_FOUND)

      if(NOT spdlog_DOWNLOAD_URL)
        get_github_latest_release_url("gabime" "spdlog")
      endif()
      # 💡Using URL is faster than git clone
      FetchContent_Declare(spdlog URL ${spdlog_DOWNLOAD_URL})
      FetchContent_MakeAvailable(spdlog)
    else()
      message(FATAL_ERROR "⛔spdlog not found, please install it or set BUILD_MISSING_DEPENDENCIES to ON")
    endif()
  endif()

  target_include_directories(${PROJECT_NAME} PUBLIC ${spdlog_INCLUDE_DIR})
  target_link_libraries(${PROJECT_NAME} PUBLIC spdlog::spdlog)
  target_compile_definitions(${PROJECT_NAME} PUBLIC TRANTOR_SPDLOG_SUPPORT)

endif()
