cmake_minimum_required(VERSION 3.24)
project(untitled3 C)

set(CMAKE_C_STANDARD 99)

# --- 1. PODEŠAVANJE ZA MAC (HOMEBREW) ---
# Ovo je ključno za M1/M2/M3 Macove da bi našao GLFW
find_program(BREW_PROG brew)
if (BREW_PROG)
    execute_process(COMMAND ${BREW_PROG} --prefix OUTPUT_VARIABLE BREW_PREFIX OUTPUT_STRIP_TRAILING_WHITESPACE)
    # Dodajemo Homebrew include i lib putanje
    include_directories(${BREW_PREFIX}/include)
    link_directories(${BREW_PREFIX}/lib)
endif()

# --- 2. DODAVANJE TVOJIH LOKALNIH HEADER-A ---
# Ovo rešava problem sa <glad/glad.h> i <cglm/cglm.h>
# ${CMAKE_SOURCE_DIR} znači tvoj glavni folder projekta
include_directories(${CMAKE_SOURCE_DIR}/include)

# --- 3. PRONALAZAK PAKETA ---
find_package(glfw3 REQUIRED)

# --- 4. KREIRANJE IZVRŠNOG FAJLA ---
# BITNO: Ovde moraju biti navedeni svi .c fajlovi
add_executable(untitled3
        src/main.c
        src/glad.c
        include/miniaudio.h
)

# --- 5. LINKOVANJE BIBLIOTEKA ---
target_link_libraries(untitled3 glfw)

# --- 6. MAC OS SPECIFIČNA LINKOVANJA ---
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