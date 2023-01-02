cmake_minimum_required(VERSION 3.21)
project(BMP_images_processing C)

set(CMAKE_C_STANDARD 11)

add_executable(BMP_images_processing sequential.c)

target_link_libraries(BMP_images_processing m)
