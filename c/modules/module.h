#pragma once

#include "../functions/functions.h"
#include "../result.h"

/**
 * The module constructor.
 *
 * Called when a module command is loaded from the shared object.
 *
 * @return  The result of the function.
 */
result module_constructor();

/**
 * The module destructor.
 *
 * Called when a module command from the shared object is removed.
 */
void module_destructor();
