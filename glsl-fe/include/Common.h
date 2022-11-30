#pragma once
#include <cassert>

#define GLSLD_DEBUG 1
#define GLSLD_DEBUG_PARSER 1

#define GLSLD_ASSERT(EXPR) assert(EXPR)

#define GLSLD_UNREACHABLE() std::abort()
#define GLSLD_NO_IMPL() std::abort()