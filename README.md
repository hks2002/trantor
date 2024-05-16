# TRANTOR

[![Build Ubuntu gcc](../../actions/workflows/ubuntu-gcc.yml/badge.svg)](../../actions/workflows/ubuntu-gcc.yml/badge.svg)
[![Build Macos clang](../../actions/workflows/macos-clang.yml/badge.svg)](../../actions/workflows/macos-clang.yml/badge.svg)
[![Build RockyLinux gcc](../../actions/workflows/rockylinux-gcc.yml/badge.svg)](../../actions/workflows/rockylinux-gcc.yml/badge.svg)
[![Build Windows msvc-vcpkg](../../actions/workflows/windows-msvc-vcpkg.yml/badge.svg)](../../actions/workflows/windows-msvc-vcpkg.yml/badge.svg)
[![Build Windows msvc-conan](../../actions/workflows/windows-msvc-conan.yml/badge.svg)](../../actions/workflows/windows-msvc-conan.yml/badge.svg)
[![Build Windows msvc-cmake-fetch](../../actions/workflows/windows-msvc-cmake-fetch.yml/badge.svg)](../../actions/workflows/windows-msvc-cmake-fetch.yml/badge.svg)

## Overview

A non-blocking I/O cross-platform TCP network library, using C++14.  
Drawing on the design of Muduo Library

## Supported platforms

- Linux
- MacOS
- UNIX(BSD)
- Windows

## Feature highlights

- Non-blocking I/O
- cross-platform
- Thread pool
- Lock free design
- Support SSL
- Server and Client

## Build

```shell
git clone https://github.com/an-tao/trantor.git
cd trantor
cmake -B build -H.
cd build
make -j
```

#### Building options

Trantor provides some building options, you can enable or disable them by setting the corresponding variables to `ON` or `OFF` in the cmake command line, cmake file etc...

| Option name                | Description                                                                                                                                      | Default value |
| :------------------------- | :----------------------------------------------------------------------------------------------------------------------------------------------- | :------------ |
| BUILD_SHARED_LIBS          | Build Trantor as a shared lib                                                                                                                    | OFF           |
| BUILD_TESTING              | Build tests                                                                                                                                      | OFF           |
| BUILD_DOC                  | Build Doxygen documentation                                                                                                                      | OFF           |
| BUILD_MISSING_DEPENDENCIES | Fetch and build dependencies, this allow you use latest or change dependencies version, if you don't want to use system package manager provided | OFF           |
| TRANTOR_USE_SPDLOG         | Allow using the spdlog logging library                                                                                                           | ON            |
| TRANTOR_USE_C-ARES         | Allow using C-ARES                                                                                                                               | ON            |
| TRANTOR_TLS_PROVIDER       | TLS provider for trantor. Valid options are 'none', 'openssl', 'botan-3', 'auto'.                                                                | auto          |

> With option `BUILD_MISSING_DEPENDENCIES` on, the related dependencies source will download and build automatically based on the options settings, this make develop more easily in `Windows`.
>
> - Build `openssl` needs `perl` and build `botan` needs `python`.
> - Passing `-DCMAKE_TOOLCHAIN_FILE="your-toolchain.cmake"` will turn off `BUILD_MISSING_DEPENDENCIES`.

> There are some scripts named start with `deps.*` for you to quickly install all dependencies also.
>
> - If you want to use system installed package manager (such as: brew, apt, dnf) provided dependencies, please turn off option `BUILD_MISSING_DEPENDENCIES`.
> - If these scripts is not fit your system, you can turn on option `BUILD_MISSING_DEPENDENCIES`.

## Licensing

Trantor - A non-blocking I/O based TCP network library, using C++14.

Copyright (c) 2016-2021, Tao An. All rights reserved.

https://github.com/an-tao/trantor

For more information see [License](License)

## Community

[Gitter](https://gitter.im/drogon-web/community)

## Documentation

[DocsForge](https://trantor.docsforge.com/)
