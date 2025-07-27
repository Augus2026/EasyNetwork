#!/bin/sh
export VCPKG_ROOT=/home/vm/EA/mtls_server/vcpkg/temp/vcpkg-export-20250727-043559
cmake -S ./ -B ./build -DCMAKE_TOOLCHAIN_FILE=/home/vm/EA/mtls_server/vcpkg/temp/vcpkg-export-20250727-043559/scripts/buildsystems/vcpkg.cmake
cmake --build ./build --config Release