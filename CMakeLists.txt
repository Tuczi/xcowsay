cmake_minimum_required(VERSION 3.15)
project(xcowsay)

set(CMAKE_CXX_STANDARD 17)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/bin")

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wpedantic -Wzero-as-null-pointer-constant\
    -Wcomment -Wdouble-promotion -Wfloat-conversion -Wfloat-equal \
    -Wformat-nonliteral -Wformat-security -Wformat-signedness \
    -Wmissing-declarations -Wshadow -Wswitch-default")

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()
set(CMAKE_CXX_FLAGS_DEBUG "-g")
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -s")

add_executable(xcowsay
        src/options.hpp
        src/vroot.h
        src/optionsFactory.cpp
        src/optionsFactory.hpp
        src/csiColors.hpp
        src/csi.hpp
        src/csiParserException.hpp
        src/csiParser.cpp
        src/csiParser.hpp
        src/main.cpp
        src/main.hpp
        src/xCowsay.cpp
        src/xCowsay.hpp
        src/xCowsayFactory.cpp
        src/xCowsayFactory.hpp)

set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,--no-undefined")
target_link_libraries(xcowsay "-lX11")
