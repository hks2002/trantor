if(BUILD_TESTING)
  message(STATUS "Setting Testing")

  find_package(GTest CONFIG)
  if(NOT GTest_FOUND)
    if(BUILD_MISSING_DEPENDENCIES)
      message(STATUS "⚠️GTest not found, Building it with FetchContent...")
      unset(GTest_FOUND)

      if(NOT googletest_DOWNLOAD_URL)
        get_github_latest_release_url("google" "googletest")
      endif()

      # Disable GMock
      set(BUILD_GMOCK OFF)

      # 💡Using URL is faster than git clone
      FetchContent_Declare(googletest URL ${googletest_DOWNLOAD_URL})
      FetchContent_MakeAvailable(googletest)

      # Set build output path, this is mainly for Windows, to solve the test/unittests running 0x000135 error
      if(WIN32)
        set_target_properties(gtest PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
        set_target_properties(gtest PROPERTIES LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
        set_target_properties(gtest PROPERTIES ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
        set_target_properties(gtest_main PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
        set_target_properties(gtest_main PROPERTIES LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
        set_target_properties(gtest_main PROPERTIES ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
      endif()

    else()
      message(FATAL_ERROR "⛔GTest not found, please install it or set BUILD_MISSING_DEPENDENCIES to ON")
    endif()
  endif()

  enable_testing()
  # unittests
  add_subdirectory(trantor/unittests)
  # tests
  add_subdirectory(trantor/tests)
endif()
