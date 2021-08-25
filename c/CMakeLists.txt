cmake_minimum_required(VERSION 3.10)
project(iron_man C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -pedantic -Werror")

add_executable(iron_man main.c
        commands/commands.c
        commands/commands.h
        commands/get_file/get_file.c
        commands/get_file/get_file.h
        commands/put_file/put_file.c
        commands/put_file/put_file.h
        commands/run_shell/run_shell.c
        commands/run_shell/run_shell.h
        communication/buffer.c
        communication/buffer.h
        communication/connection.c
        communication/connection.h
        consts.h
        logging/logging.c
        logging/logging.h
        result.h
        tiny-aes/aes.c
        tiny-aes/aes.h)
