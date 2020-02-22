cmake_minimum_required(VERSION 3.5)
project(xcowsay)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

set(COMPILATION_WARNINGS "-Wall -Wextra -Wpedantic -Wzero-as-null-pointer-constant\
    -Wcomment -Wdouble-promotion -Wfloat-conversion -Wfloat-equal \
    -Wformat-nonliteral -Wformat-security -Wformat-signedness \
    -Wmissing-declarations -Wshadow -Wswitch-default")

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14 ${COMPILATION_WARNINGS} -O3 -s")

set(SOURCE_FILES
        src/args_parser.cpp
        src/csi.cpp
        src/main.cpp
        src/xcowsay.cpp)

set(HEADER_FILES
        src/csi.hpp
        src/args_parser.hpp
        src/main.hpp
        src/vroot.h
        src/xcowsay.hpp
        src/xterm_colors.hpp)

add_executable(xcowsay ${SOURCE_FILES} ${HEADER_FILES})
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,--no-undefined -lX11")
target_link_libraries(xcowsay "-lX11")
