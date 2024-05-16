message(STATUS "Trantor using SSL library: botan")
target_compile_definitions(${PROJECT_NAME} PRIVATE TRANTOR_TLS_PROVIDER="Botan")
target_compile_definitions(${PROJECT_NAME} PRIVATE USE_BOTAN)

# conan target is different, always lower case
if(TARGET botan::botan)
  target_link_libraries(${PROJECT_NAME} PRIVATE botan::botan)
  target_compile_features(botan::botan INTERFACE cxx_std_20)
else()
  target_link_libraries(${PROJECT_NAME} PRIVATE Botan::Botan)
endif()

list(
  APPEND
  TRANTOR_SOURCES
  trantor/net/inner/tlsprovider/BotanTLSProvider.cc
  trantor/utils/crypto/botan.cc
)
