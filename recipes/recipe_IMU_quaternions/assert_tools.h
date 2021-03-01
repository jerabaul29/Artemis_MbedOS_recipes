#ifndef ASSERT_TOOLS
#define ASSERT_TOOLS

#include "Arduino.h"

// user assert
void uassert(bool condition, const char * msg="fail assert, abort");

#endif
