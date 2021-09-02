#pragma once

#include "../../communication/buffer.h"
#include "../../communication/connection.h"
#include "../../result.h"

#include <signal.h>
#include <sys/wait.h>

buffer run_shell(result *res, buffer *buf);
