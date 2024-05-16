message(STATUS "Trantor using SSL library: openssl")

target_compile_definitions(${PROJECT_NAME} PRIVATE TRANTOR_TLS_PROVIDER="OpenSSL")
target_compile_definitions(${PROJECT_NAME} PRIVATE USE_OPENSSL)

target_link_libraries(${PROJECT_NAME} PRIVATE OpenSSL::SSL OpenSSL::Crypto)

if(WIN32)
  target_link_libraries(${PROJECT_NAME} PRIVATE Crypt32 Secur32)
endif()

list(APPEND TRANTOR_SOURCES trantor/net/inner/tlsprovider/OpenSSLProvider.cc)
