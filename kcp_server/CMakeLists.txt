cmake_minimum_required(VERSION 3.10)
# 项目名称
project(kcp_server)

# 设置 C 标准
set(CMAKE_C_STANDARD 99)

find_package(kcp REQUIRED)

set(UTHASH_INCLUDE_DIR "${VCPKG_ROOT}/installed/x64-windows/include")
include_directories(kcp_server ${UTHASH_INCLUDE_DIR})

# 添加客户端可执行文件
add_executable(kcp_server
    main.c
    message.h
    client_mgr.h
    client_mgr.c
    udp_server.h
    udp_server.c
    kcp_utils.h
)

# 链接 Winsock 库
target_link_libraries(kcp_server
    kcp::kcp
    ws2_32
    winmm.lib
)

if(MSVC)
    add_compile_options(/utf-8)
else()
    add_compile_options(-finput-charset=UTF-8 -fexec-charset=UTF-8)
endif()