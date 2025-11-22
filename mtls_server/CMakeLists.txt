cmake_minimum_required(VERSION 3.10)
project(mtls_server C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

find_package(wolfssl REQUIRED)

set(CMAKE_EXE_LINKER_FLAGS "-static")

include_directories(log.c/src)

add_executable(mtls_server
    main.c
    client_mgr.h
    client_mgr.c
    stage.h
    stage.c
    message.h
    user_settings.h
    user_settings_dtls.h
    log.c/src/log.c
)

find_path(UTHASH_INCLUDE_DIRS "utarray.h")
target_include_directories(mtls_server PRIVATE
    ${UTHASH_INCLUDE_DIRS}
)

target_compile_definitions(mtls_server PRIVATE
    LOG_USE_COLOR
)

if(WIN32)
    target_compile_definitions(mtls_server PRIVATE
        WOLFSSL_USER_SETTINGS
    )

    target_link_libraries(mtls_server
        wolfssl::wolfssl
        ws2_32)
else()
    target_link_libraries(mtls_server
        wolfssl::wolfssl
        pthread)
endif()