#ifndef EMSCR_H
#define EMSCR_H

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#include <emscripten/html5.h>
#else
#include <stdbool.h>
#define EM_TRUE true
#define EM_BOOL bool
#endif

#endif//EMSCR_H
