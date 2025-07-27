cmake_minimum_required(VERSION 3.10)
project(EasyNetwork C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

set(WINTUN_DIR "${CMAKE_SOURCE_DIR}/wintun")
include_directories(EasyNetwork ${WINTUN_DIR}/include)

find_package(wolfssl REQUIRED)
find_package(kcp REQUIRED)

# 添加manifest文件
set(MANIFEST_FILE "${CMAKE_SOURCE_DIR}/app.manifest")
# 添加version.rc文件
set(VERSION_RC "${CMAKE_SOURCE_DIR}/app.rc")

set(EASY_NETWORK_SOURCES
    easy_network_ffi.h
    easy_network_ffi.c
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

    kcp_client.h
    kcp_client.c
    udp_client.h
    udp_client.c
    message.h
    kcp_utils.h
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
    LINK_DEPENDS ${MANIFEST_FILE}
    LINK_FLAGS "/MANIFEST:NO /MANIFESTINPUT:${MANIFEST_FILE}"
)

# 链接库
target_link_libraries(EasyNetwork
    wolfssl::wolfssl
    kcp::kcp
    iphlpapi
    ws2_32
    ntdll
    rpcrt4
)

# 为 EasyNetwork 目标添加编译参数
target_compile_options(EasyNetwork PRIVATE /utf-8)

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

add_custom_command(TARGET EasyNetwork POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/ca
        $<TARGET_FILE_DIR:EasyNetwork>
)

# 添加 peera.c 作为可执行目标
add_executable(peera peera.c)
target_link_libraries(peera EasyNetwork)
add_custom_command(TARGET peera POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<TARGET_FILE:EasyNetwork>
        $<TARGET_FILE_DIR:peera>
)

# 添加 peerb.c 作为可执行目标
add_executable(peerb peerb.c)
target_link_libraries(peerb EasyNetwork)
add_custom_command(TARGET peerb POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<TARGET_FILE:EasyNetwork>
        $<TARGET_FILE_DIR:peerb>
)