cmake_minimum_required(VERSION 3.24)
project(untitled3 C)

set(CMAKE_C_STANDARD 99)


find_program(BREW_PROG brew)
if (BREW_PROG)
    execute_process(COMMAND ${BREW_PROG} --prefix OUTPUT_VARIABLE BREW_PREFIX OUTPUT_STRIP_TRAILING_WHITESPACE)
    include_directories(${BREW_PREFIX}/include)
    link_directories(${BREW_PREFIX}/lib)
endif()

include_directories(${CMAKE_SOURCE_DIR}/include)


find_package(glfw3 REQUIRED)

add_executable(untitled3
        src/main.c
        src/glad.c
        include/miniaudio.h
)


target_link_libraries(untitled3 glfw)


if (APPLE)
    target_link_libraries(untitled3 "-framework OpenGL")
    target_link_libraries(untitled3 "-framework Cocoa")
    target_link_libraries(untitled3 "-framework IOKit")
    target_link_libraries(untitled3 "-framework CoreVideo")
endif()
if(APPLE)
    target_link_libraries(untitled3 "-framework CoreAudio -framework AudioToolbox -framework CoreFoundation")
elseif(UNIX AND NOT APPLE)
    target_link_libraries(untitled3 m dl pthread)
endif()
