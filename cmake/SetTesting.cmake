if(BUILD_TESTING)
  message(STATUS "Setting Testing")
  # unittests
  enable_testing()
  add_subdirectory(trantor/unittests)
  # tests
  add_subdirectory(trantor/tests)
endif()
