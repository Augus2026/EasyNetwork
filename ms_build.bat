@echo off
setlocal

:: Check if project parameter is provided
if "%~1"=="" (
    echo Please specify the project to build:
    echo   - lib: Build EasyNetwork library
    echo   - flutter: Build Flutter application
    echo   - api_server: Build API server
    echo   - mtls_server: Build mTLS server
    goto :eof
)

:: Select build project based on parameter
if /i "%~1"=="lib" (
    echo Building EasyNetwork library...

    pushd .\EasyNetwork\libEasyNetwork
    cmake -S ./ -B ./build -DCMAKE_TOOLCHAIN_FILE=C:/ws/dev/vcpkg/temp/vcpkg-export-20250921-180547/scripts/buildsystems/vcpkg.cmake
    cmake --build ./build --config Debug
    cmake --build ./build --config Release
    popd
) else if /i "%~1"=="flutter" (
    echo Building EasyNetwork library...

    pushd .\EasyNetwork\flutter
    @REM flutter pub run ffigen --config ffigen.yaml
    flutter pub get
    flutter build windows --release
    popd
) else if /i "%~1"=="api_server" (
    echo Building API server...

    pushd .\api_server
    cargo clean
    cargo build --release
    popd
) else if /i "%~1"=="mtls_server" (
    echo Building mTLS server...

    pushd .\mtls_server
    cmake -S ./ -B ./build -DCMAKE_TOOLCHAIN_FILE=C:/ws/dev/vcpkg/temp/vcpkg-export-20250921-180547/scripts/buildsystems/vcpkg.cmake
    cmake --build ./build --config Release
    popd
) else (
    echo Invalid project parameter: %~1
    echo Available options: flutter, lib, api_server, mtls_server
)