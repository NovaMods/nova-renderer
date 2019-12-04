#pragma once

#include <X11/Xlib.h>

// X11 macros that are bad
#ifdef Always
#undef Always
#endif

#ifdef None
#undef None
#endif

#ifdef Bool
#undef Bool
#endif
