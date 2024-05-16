add_executable(split_string_unittest splitStringUnittest.cc)
add_executable(string_encoding_unittest stringEncodingUnittest.cc)
add_executable(date_unittest DateUnittest.cc)
add_executable(hash_unittest HashUnittest.cc)
add_executable(inetaddress_unittest InetAddressUnittest.cc)
add_executable(msgbuffer_unittest MsgBufferUnittest.cc)

set(UNITTEST_TARGETS
    split_string_unittest
    string_encoding_unittest
    date_unittest
    hash_unittest
    inetaddress_unittest
    msgbuffer_unittest
)

if(TRANTOR_USE_TLS)
  add_executable(ssl_name_verify_unittest sslNameVerifyUnittest.cc)
  list(APPEND UNITTEST_TARGETS ssl_name_verify_unittest)
endif()

set_property(TARGET ${UNITTEST_TARGETS} PROPERTY CXX_STANDARD 17)
set_property(TARGET ${UNITTEST_TARGETS} PROPERTY CXX_STANDARD_REQUIRED ON)
set_property(TARGET ${UNITTEST_TARGETS} PROPERTY CXX_EXTENSIONS OFF)

include(GoogleTest)

foreach(T ${UNITTEST_TARGETS})
  message(STATUS "Setting Testing ${T}")
  target_link_libraries(${T} PRIVATE ${PROJECT_NAME} GTest::gtest_main)

  gtest_discover_tests(${T})
endforeach()
