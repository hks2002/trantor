cmake_minimum_required(VERSION 3.11)
project(
  trantor
  DESCRIPTION "Non-blocking I/O cross-platform TCP network library, using C++17"
  HOMEPAGE_URL "https://github.com/an-tao/trantor"
)
add_library(${PROJECT_NAME})

# Version
set(TRANTOR_MAJOR_VERSION 1)
set(TRANTOR_MINOR_VERSION 5)
set(TRANTOR_PATCH_VERSION 20)
set(TRANTOR_VERSION ${TRANTOR_MAJOR_VERSION}.${TRANTOR_MINOR_VERSION}.${TRANTOR_PATCH_VERSION})

# C++ standard
set_target_properties(${PROJECT_NAME} PROPERTIES CXX_STANDARD 17)
set_target_properties(${PROJECT_NAME} PROPERTIES CXX_STANDARD_REQUIRED ON)
set_target_properties(${PROJECT_NAME} PROPERTIES CXX_EXTENSIONS OFF)

# Build options
include(cmake/SetBuildOptions.cmake)
if(BUILD_SHARED_LIBS)
  set_target_properties(
    ${PROJECT_NAME}
    PROPERTIES
    VERSION ${TRANTOR_VERSION}
    SOVERSION ${TRANTOR_MAJOR_VERSION}
  )
endif()

# Sources and headers
set(PUBLIC_HEADERS # cmake-format: sortable
                   trantor/Callbacks.h trantor/NonCopyable.h trantor/ObjectPool.h
)

set(PUBLIC_UTILS_HEADERS
    # cmake-format: sortable
    trantor/utils/Date.h
    trantor/utils/Encoding.h
    trantor/utils/Hash.h
    trantor/utils/StringUtils.h
)

set(PUBLIC_LOGGER_HEADERS
    # cmake-format: sortable
    trantor/logger/AsyncFileLogger.h
    trantor/logger/FixedBuffer.h
    trantor/logger/Fmt.h
    trantor/logger/Logger.h
    trantor/logger/LoggerFile.h
    trantor/logger/LogStream.h
    trantor/logger/RawLogger.h
    trantor/logger/SourceFile.h
)

set(PUBLIC_NET_HEADERS
    # cmake-format: sortable
    trantor/net/Acceptor.h
    trantor/net/AsyncStream.h
    trantor/net/Certificate.h
    trantor/net/Connector.h
    trantor/net/core/Channel.h
    trantor/net/core/ConcurrentTaskQueue.h
    trantor/net/core/EventLoop.h
    trantor/net/core/EventLoopThread.h
    trantor/net/core/EventLoopThreadPool.h
    trantor/net/core/LockFreeQueue.h
    trantor/net/core/poller/EpollPoller.h
    trantor/net/core/poller/KQueue.h
    trantor/net/core/poller/Poller.h
    trantor/net/core/poller/PollPoller.h
    trantor/net/core/SerialTaskQueue.h
    trantor/net/core/TaskQueue.h
    trantor/net/core/Timer.h
    trantor/net/core/TimerQueue.h
    trantor/net/core/TimingWheel.h
    trantor/net/InetAddress.h
    trantor/net/MsgBuffer.h
    trantor/net/resolver/Resolver.h
    trantor/net/Socket.h
    trantor/net/TcpClient.h
    trantor/net/TcpConnection.h
    trantor/net/TcpConnectionImpl.h
    trantor/net/TcpServer.h
    trantor/net/TLSPolicy.h
)

set(TRANTOR_SOURCES
    # cmake-format: sortable
    trantor/logger/AsyncFileLogger.cc
    trantor/logger/FixedBuffer.cc
    trantor/logger/Fmt.cc
    trantor/logger/Logger.cc
    trantor/logger/LoggerFile.cc
    trantor/logger/LogStream.cc
    trantor/logger/RawLogger.cc
    trantor/net/Acceptor.cc
    trantor/net/buffernode/AsyncStreamBufferNode.cc
    trantor/net/buffernode/MemBufferNode.cc
    trantor/net/buffernode/StreamBufferNode.cc
    trantor/net/Connector.cc
    trantor/net/core/Channel.cc
    trantor/net/core/ConcurrentTaskQueue.cc
    trantor/net/core/EventLoop.cc
    trantor/net/core/EventLoopThread.cc
    trantor/net/core/EventLoopThreadPool.cc
    trantor/net/core/poller/EpollPoller.cc
    trantor/net/core/poller/KQueue.cc
    trantor/net/core/poller/Poller.cc
    trantor/net/core/poller/PollPoller.cc
    trantor/net/core/SerialTaskQueue.cc
    trantor/net/core/Timer.cc
    trantor/net/core/TimerQueue.cc
    trantor/net/core/TimingWheel.cc
    trantor/net/InetAddress.cc
    trantor/net/MsgBuffer.cc
    trantor/net/Socket.cc
    trantor/net/TcpClient.cc
    trantor/net/TcpConnectionImpl.cc
    trantor/net/TcpServer.cc
    trantor/utils/Date.cc
    trantor/utils/Encoding.cc
    trantor/utils/Hash.cc
    trantor/utils/StringUtils.cc
)

if(WIN32)
  list(
    APPEND
    PRIVATE_HEADERS
    # cmake-format: sortable
    third_party/wepoll/Wepoll.h
    trantor/net/WindowsSupport.h
  )
  list(
    APPEND
    TRANTOR_SOURCES
    # cmake-format: sortable
    third_party/wepoll/Wepoll.c
    trantor/net/WindowsSupport.cc
    trantor/net/buffernode/FileBufferNodeWin.cc
  )
else()
  list(APPEND TRANTOR_SOURCES trantor/net/buffernode/FileBufferNodeUnix.cc)
endif()

# Export header
include(cmake/SetExportHeader.cmake)

# Target include directories
target_include_directories(
  ${PROJECT_NAME}
  PUBLIC $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}> $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/exports> $<INSTALL_INTERFACE:${INSTALL_INCLUDE_DIR}>
  PRIVATE ${PROJECT_SOURCE_DIR}
          ${PROJECT_SOURCE_DIR}/trantor
          ${PROJECT_SOURCE_DIR}/trantor/logger
          ${PROJECT_SOURCE_DIR}/trantor/utils
          ${PROJECT_SOURCE_DIR}/trantor/net
          ${PROJECT_SOURCE_DIR}/trantor/net/core
          ${PROJECT_SOURCE_DIR}/trantor/net/inner
          $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/third_party/wepoll>
)

# CMake modules
list(APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake_modules/)

# Common settings
include(cmake/SetSystemAndCompiler.cmake)
include(cmake/SetThreads.cmake)

# Set options
include(cmake/SetOptionsDependencies.cmake)
include(cmake/SetSpdlog.cmake)
include(cmake/SetCAres.cmake)
include(cmake/SetTlsProvider.cmake)
include(cmake/SetTesting.cmake)
include(cmake/SetDoxygen.cmake)

# Group sources
source_group(
  "Public API"
  FILES ${PUBLIC_HEADERS}
        ${PUBLIC_LOGGER_HEADERS}
        ${PUBLIC_UTILS_HEADERS}
        ${PUBLIC_NET_HEADERS}
        ${TRANTOR_EXPORT_HEADER}
)

# Target sources
target_sources(
  ${PROJECT_NAME}
  PRIVATE ${PUBLIC_HEADERS}
          ${PUBLIC_LOGGER_HEADERS}
          ${PUBLIC_UTILS_HEADERS}
          ${PUBLIC_NET_HEADERS}
          ${PRIVATE_HEADERS}
          ${TRANTOR_EXPORT_HEADER}
          ${TRANTOR_SOURCES}
)

# Set additional
include(cmake/SetInstall.cmake)
include(cmake/SetPackageConfig.cmake)
