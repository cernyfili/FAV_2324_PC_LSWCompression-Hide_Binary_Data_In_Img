cmake_minimum_required(VERSION 3.22)
project(FAV_PC_SP_23_24_BinaryDataInImg C)

set(CMAKE_C_STANDARD 99)

add_executable(FAV_PC_SP_23_24_BinaryDataInImg
        src/main.c
        src/compression.c
        src/compression.h
        src/image.c
        src/image.h
        src/payload.c
        src/payload.h
        src/utils/logger.c
        src/utils/logger.h
        src/utils/memory_management.c
        src/utils/memory_management.h
        src/utils/utils.h
        src/utils/data_structures.c
        src/utils/data_structures.h
        src/utils/dictionary.c
        src/utils/dictionary.h
        src/utils/utils.c
        src/test/test_file.c
        src/test/test_file.h
)

find_package(PNG REQUIRED)
include_directories(${PNG_INCLUDE_DIRS})

target_link_libraries(FAV_PC_SP_23_24_BinaryDataInImg ${PNG_LIBRARIES})




