#pragma once

#include <protec/skCrypt.h>

#ifdef UTN_DEV_CONSOLE
#define UTN_DEV_LOG(statement) do { statement; } while (0)
#else
#define UTN_DEV_LOG(statement) do {} while (0)
#endif
