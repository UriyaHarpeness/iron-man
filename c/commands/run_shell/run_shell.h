#pragma once

#include "../../communication/buffer.h"
#include "../../communication/connection.h"
#include "../../result.h"

#include <signal.h>
#include <sys/wait.h>

#define RUN_SHELL_COMMAND_ID 0x2385d0791aec41e3

buffer run_shell(result *res, buffer *buf);
