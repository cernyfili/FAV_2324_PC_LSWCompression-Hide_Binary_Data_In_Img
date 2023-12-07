cmake_minimum_required(VERSION 3.22)
project(FAV_PC_SP_23_24_BinaryDataInImg C)

set(CMAKE_C_STANDARD 99)

add_executable(FAV_PC_SP_23_24_BinaryDataInImg src/main.c src/utils/logger.c src/utils/logger.h src/payload_prepare.c src/payload_prepare.h src/utils/memory_management.c src/utils/memory_management.h src/utils/utils.h
        src/utils/data_structures.c
        src/utils/data_structures.h)

find_package(PNG REQUIRED)
include_directories(${PNG_INCLUDE_DIRS})

target_link_libraries(my_executable ${PNG_LIBRARIES})




