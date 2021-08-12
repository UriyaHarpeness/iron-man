#pragma once

#include "buffer.h"
#include "../logging/logging.h"
#include "../result.h"

#include <netdb.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#define PORT 8080

result communicate();

result connect_();

void disconnect();
