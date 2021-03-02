cmake_minimum_required(VERSION 3.10)
project(puissance4 C)

set(CMAKE_C_STANDARD 11)
add_compile_options(-O3)

add_executable(puissance4
        src/jeu.c)
target_link_libraries(puissance4 m)
