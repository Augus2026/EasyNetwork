@echo off

pushd .\EasyNetwork\libEasyNetwork
cmake -S ./ -B ./build -DCMAKE_TOOLCHAIN_FILE=C:/Users/shiguoliang/temp/vcpkg-export-20250726-155106/scripts/buildsystems/vcpkg.cmake
cmake --build ./build --config Release
popd

pushd .\EasyNetwork\flutter
@REM flutter pub run ffigen --config ffigen.yaml
@REM flutter pub get
flutter build windows --release
popd

@REM pushd .\api_server
@REM cargo clean
@REM cargo build --release
@REM popd

@REM pushd .\mtls_server
@REM cmake -S ./ -B ./build -DCMAKE_TOOLCHAIN_FILE=C:/Users/shiguoliang/temp/vcpkg-export-20250726-155106/scripts/buildsystems/vcpkg.cmake
@REM cmake --build ./build --config Release
@REM popd