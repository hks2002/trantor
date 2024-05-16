# Set TLS provider
message(STATUS "Setting TLS: ${TRANTOR_USE_TLS}  TRANTOR_TLS_PROVIDER: ${TRANTOR_TLS_PROVIDER}")

# Checking valid TLS providers
set(VALID_TLS_PROVIDERS
    "none"
    "openssl"
    "botan-3"
    "auto"
)
if(NOT
   "${TRANTOR_TLS_PROVIDER}"
   IN_LIST
   VALID_TLS_PROVIDERS
)
  message(FATAL_ERROR "Invalid TLS provider: ${TRANTOR_TLS_PROVIDER}\n"
                      "Valid TLS providers are: ${VALID_TLS_PROVIDERS}"
  )
endif()

if(TRANTOR_TLS_PROVIDER STREQUAL "openssl") # openssl
  find_package(OpenSSL REQUIRED)
  include(cmake/SetOpenssl.cmake)

elseif(TRANTOR_TLS_PROVIDER STREQUAL "botan-3") # botan 3
  find_package(Botan 3 REQUIRED)

  include(cmake/SetBotan.cmake)

elseif(TRANTOR_TLS_PROVIDER STREQUAL "none") # none

  include(cmake/SetCrypto.cmake)

elseif(TRANTOR_TLS_PROVIDER STREQUAL "auto") # auto
  find_package(OpenSSL)

  if(OPENSSL_FOUND)

    include(cmake/SetOpenssl.cmake)
  else()
    find_package(Botan 3)

    if(BOTAN_FOUND)
      include(cmake/SetBotan.cmake)
    else()
      include(cmake/SetCrypto.cmake)
    endif() # botan 3

  endif() # openssl

else() # none
  include(cmake/SetCrypto.cmake)
endif()
