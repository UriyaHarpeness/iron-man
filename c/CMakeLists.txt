cmake_minimum_required(VERSION 3.10)
project(iron_man C)

set(CMAKE_C_STANDARD 11)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -pedantic -Werror")
set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -DDEBUG_BUILD")
set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -DRELEASE_BUILD")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -T ${CMAKE_SOURCE_DIR}/custom_sections.lds")

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
        functions/functions.c
        functions/functions.h
        logging/logging.c
        logging/logging.h
        result.h
        tiny-aes/aes.c
        tiny-aes/aes.h)

target_link_libraries(iron_man ${CMAKE_DL_LIBS})

add_custom_command(OUTPUT consts.h
        DEPENDS consts.h.ini
        COMMAND PYTHONPATH=../../python python3 ../../python/building/generate_consts.py
        --template ${CMAKE_SOURCE_DIR}/consts.h.ini
        --config ../../python/config.json)

add_custom_command(OUTPUT functions/functions.h
        DEPENDS functions/functions.h.ini
        COMMAND PYTHONPATH=../../python python3 ../../python/building/generate_functions_loading.py
        --template ${CMAKE_SOURCE_DIR}/functions/functions.h.ini)

add_custom_command(TARGET iron_man
        POST_BUILD
        COMMAND PYTHONPATH=../../python python3 ../../python/building/post_build.py
        --executable $<TARGET_FILE:iron_man>
        --config ../../python/config.json)

if (CMAKE_BUILD_TYPE MATCHES Release)
    add_custom_command(TARGET iron_man
            POST_BUILD
            COMMAND strip -s $<TARGET_FILE:iron_man>)
endif ()
