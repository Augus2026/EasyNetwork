cmake_minimum_required(VERSION 3.10)
project(EasyNetwork C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

set(WINTUN_DIR "${CMAKE_SOURCE_DIR}/wintun")
include_directories(EasyNetwork ${WINTUN_DIR}/include)

find_package(wolfssl REQUIRED)

set(EASY_NETWORK_SOURCES
    easynet_ffi.h
    easynet_ffi.c
    itf.h
    itf.c
    wintun_peer.h
    wintun_peer.c
    wintun_init.h
    wintun_init.c
    tls_client.h
    tls_client.c
    user_settings.h
    user_settings_dtls.h
    logc/log.h
    logc/log.c
)

# 添加库目标
add_library(EasyNetwork SHARED
    ${EASY_NETWORK_SOURCES}
    ${MANIFEST_FILE}
    ${VERSION_RC}
)

# 设置库目标属性
set_target_properties(EasyNetwork PROPERTIES
    OUTPUT_NAME "EasyNetwork"
    PREFIX "lib"
)

# 链接库
target_link_libraries(EasyNetwork
    wolfssl::wolfssl
    iphlpapi
    ws2_32
    ntdll
    rpcrt4
)

# 为 EasyNetwork 目标添加编译参数
target_compile_options(EasyNetwork PRIVATE /utf-8 -DLOG_USE_COLOR)

# 修改为控制台子系统
if(MSVC)
    target_link_options(EasyNetwork PRIVATE "/SUBSYSTEM:CONSOLE")
else()
    target_link_options(EasyNetwork PRIVATE "-Wl,-subsystem,console")
endif()

add_custom_command(TARGET EasyNetwork POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${WINTUN_DIR}/bin/amd64/wintun.dll
        $<TARGET_FILE_DIR:EasyNetwork>
)

add_custom_command(TARGET EasyNetwork POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<TARGET_FILE:wolfssl::wolfssl>
        $<TARGET_FILE_DIR:EasyNetwork>
)

# 添加 peer_tests.c 作为可执行目标
find_package(cJSON CONFIG REQUIRED)
add_executable(peer_tests
    tests/peer_tests.c
    )
target_include_directories(peer_tests PRIVATE
    ${CMAKE_SOURCE_DIR}/
    cJSON
)
target_link_libraries(peer_tests
    EasyNetwork
    cjson
)
add_custom_command(TARGET peer_tests POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<TARGET_FILE:EasyNetwork>
        $<TARGET_FILE_DIR:peer_tests>
)
