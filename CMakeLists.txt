cmake_minimum_required(VERSION 3.22)
project(FAV_PC_SP_23_24_BinaryDataInImg C)

set(CMAKE_C_STANDARD 99)

add_executable(FAV_PC_SP_23_24_BinaryDataInImg src/main.c src/image.c src/image.h src/compress.c src/compress.h src/data_hider.c src/data_hider.h src/decompress.c src/decompress.h src/steganograph.c src/steganograph.h)
