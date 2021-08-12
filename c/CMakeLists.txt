cmake_minimum_required(VERSION 3.10)
project(iron_man C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -pedantic -Werror")


add_executable(iron_man main.c
        communication/buffer.c
        communication/buffer.h
        communication/connection.c
        communication/connection.h
        logging/logging.c
        logging/logging.h
        result.h)
