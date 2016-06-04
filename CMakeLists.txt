cmake_minimum_required(VERSION 3.5)
project(xcowsay)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14 -Wall -s")

set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,--no-undefined -L/usr/lib -lX11")

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

#TODO try to use relative path and in install.sh use "make install DESTDIR=/usr/lib" "make install DESTDIR=/usr/share" with proper targets or find "install config"
install(TARGETS xcowsay
        DESTINATION /usr/lib/xscreensaver)

install(FILES xscreen-config/xcowsay.xml
        DESTINATION /usr/share/xscreensaver/config
        PERMISSIONS OWNER_WRITE OWNER_READ GROUP_READ WORLD_READ)

install(FILES cows/christmas-tree.cow
        DESTINATION /usr/share/cows
        PERMISSIONS OWNER_WRITE OWNER_READ GROUP_READ WORLD_READ)
