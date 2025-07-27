@echo off

@REM pushd .\EasyNetwork\libEasyNetwork
@REM cmake -S ./ -B ./build -DCMAKE_TOOLCHAIN_FILE=C:/Users/shiguoliang/temp/vcpkg-export-20250726-155106/scripts/buildsystems/vcpkg.cmake
@REM cmake --build ./build --config Release
@REM popd

@REM pushd .\EasyNetwork\flutter
@REM @REM flutter pub run ffigen --config ffigen.yaml
@REM @REM flutter pub get
@REM flutter build windows --release
@REM popd

@REM pushd .\api_server
@REM cargo clean
@REM cargo build --release
@REM popd

pushd .\mtls_server
cmake -S ./ -B ./build -DCMAKE_TOOLCHAIN_FILE=C:/Users/shiguoliang/temp/vcpkg-export-20250726-155106/scripts/buildsystems/vcpkg.cmake
cmake --build ./build --config Release
popd