
cmake_minimum_required(VERSION 3.10)
project(mtls_server C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

find_package(wolfssl REQUIRED)

#
if(WIN32)
    set(UTHASH_INCLUDE_DIR "${VCPKG_ROOT}/installed/x64-windows/include")
    include_directories(mtls_server ${UTHASH_INCLUDE_DIR})
else()
    set(VCPKG_ROOT "/home/vm/EA/mtls_server/vcpkg/temp/vcpkg-export-20250727-043559")
    set(UTHASH_INCLUDE_DIR "${VCPKG_ROOT}/installed/x64-linux/include")
    include_directories(mtls_server ${UTHASH_INCLUDE_DIR})
endif()

add_executable(mtls_server
    main.c
    client_mgr.h
    client_mgr.c
    stage.h
    stage.c
    message.h
    user_settings.h
    user_settings_dtls.h
    )


if(WIN32)
    target_link_libraries(mtls_server
        wolfssl::wolfssl
        ws2_32)
else()
    target_link_libraries(mtls_server
        wolfssl::wolfssl
        pthread)
endif()

add_custom_command(TARGET mtls_server POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<TARGET_FILE:wolfssl::wolfssl>
        $<TARGET_FILE_DIR:mtls_server>
)

# 拷贝ca目录下的所有文件到输出目录
add_custom_command(TARGET mtls_server POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/ca
        $<TARGET_FILE_DIR:mtls_server>
)