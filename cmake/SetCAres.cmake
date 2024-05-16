# Set normal resolver
macro(set_normal_resolver)
  list(APPEND TRANTOR_SOURCES trantor/net/resolver/NormalResolver.cc)
  list(APPEND PRIVATE_HEADERS trantor/net/resolver/NormalResolver.h)
endmacro()

# Set cares resolver
macro(set_cares_resolver)
  target_link_libraries(${PROJECT_NAME} PRIVATE c-ares::cares)

  if(APPLE)
    target_link_libraries(${PROJECT_NAME} PRIVATE resolv)
  elseif(WIN32)
    target_link_libraries(${PROJECT_NAME} PRIVATE Iphlpapi)
  endif()

  if(NOT BUILD_SHARED_LIBS)
    target_compile_definitions(${PROJECT_NAME} PRIVATE CARES_STATICLIB)
  endif()

  list(APPEND TRANTOR_SOURCES trantor/net/resolver/AresResolver.cc)
  list(APPEND PRIVATE_HEADERS trantor/net/resolver/AresResolver.h)

endmacro()

# ##################################################################################################################################################################################
message(STATUS "Setting c-ares: ${TRANTOR_USE_C-ARES}")

if(TRANTOR_USE_C-ARES)

  find_package(c-ares)
  if(NOT c-ares_FOUND)
    if(BUILD_MISSING_DEPENDENCIES)
      message(STATUS "⚠️c-ares not found, Building it with FetchContent...")
      unset(c-ares_FOUND)

      if(NOT c-ares_DOWNLOAD_URL)
        get_github_latest_release_url("c-ares" "c-ares")
      endif()
      # 💡Using URL is faster than git clone
      FetchContent_Declare(c-ares URL ${c-ares_DOWNLOAD_URL})
      FetchContent_MakeAvailable(c-ares)
    else()
      message(FATAL_ERROR "⛔c-ares not found, please install it or set BUILD_MISSING_DEPENDENCIES to ON")
    endif()
  endif()

  set_cares_resolver()
else()
  set_normal_resolver()
endif()
