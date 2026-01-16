cmake_minimum_required(VERSION 3.26.5)

project(wsl_mon C)

set(CMAKE_C_STANDARD 23)

find_package(PkgConfig REQUIRED)
pkg_check_modules(GTK REQUIRED IMPORTED_TARGET gtk4)

add_compile_options(
        -Wno-unknown-pragmas
        -Wall
        -Wextra
        -Werror
        -Wformat=2
        -Wmissing-declarations
        -Wswitch-default
        -Wunused
        -Wcast-align
        -Wfloat-equal
        -Wformat-security
        -Wconversion
        -Wpedantic
)

add_executable(wsl_mon
        src/main.c
        src/usb.c
        src/ui.c)

target_include_directories(wsl_mon PRIVATE include)

target_link_libraries(wsl_mon PRIVATE PkgConfig::GTK)

add_executable(unit_tests
        src/usb.c
        tests/usb.c
)

target_include_directories(unit_tests PRIVATE include)

target_link_libraries(unit_tests PRIVATE PkgConfig::GTK)
