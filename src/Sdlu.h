/*
 * SDL2 utility and wrapper functions.
 */

#ifndef SDLU_H
#define SDLU_H

#include <SDL2/SDL.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Functions in alphabetical order */

// Call corresponding SDL function.
// If error, print to `stderr` and exit(1).
SDL_Renderer *Sdlu_CreateRenderer(
    SDL_Window *const window,
    const int index,
    const Uint32 flags);

// Call corresponding SDL function.
// If error, print to `stderr` and exit(1).
SDL_Window *Sdlu_CreateWindow(
    const char *const title,
    const int x, const int y,
    const int w, const int h,
    const Uint32 flags);

// Call corresponding SDL function.
// If error, print to `stderr` and exit(1).
void Sdlu_CreateWindowAndRenderer(
    const int width,
    const int height,
    const Uint32 window_flags,
    SDL_Window **window,
    SDL_Renderer **renderer);

// Call corresponding SDL function.
// If error, print to `stderr` and exit(1).
void Sdlu_GetDesktopDisplayMode(
    const int displayIndex, SDL_DisplayMode *const mode);

// Call corresponding SDL function.
// If error, print to `stderr` and exit(1).
SDL_Renderer *Sdlu_GetRenderer(SDL_Window *const window);

// Call corresponding SDL function.
// If error, print to `stderr` and exit(1).
void Sdlu_GetRendererOutputSize(
    SDL_Renderer *const renderer,
    int *const w,
    int *const h);

// Call corresponding SDL function.
// If error, print to `stderr` and exit(1).
int Sdlu_GetWindowDisplayIndex(SDL_Window *const window);

// Call corresponding SDL function.
// If error, print to `stderr` and exit(1).
void Sdlu_Init(Uint32 flags);

// Call corresponding SDL function.
// If error, print to `stderr` and exit(1).
void Sdlu_RenderDrawPoint(
    SDL_Renderer *const renderer, const int x, const int y);

// Call corresponding SDL function.
// If error, print to `stderr` and exit(1).
void Sdlu_RenderFillRect(
    SDL_Renderer *const renderer, const SDL_Rect *const rect);

// Call corresponding SDL function.
// If error, print to `stderr` and exit(1).
void Sdlu_SetRenderDrawColor(
    SDL_Renderer *const renderer,
    const Uint8 r, const Uint8 g, const Uint8 b, const Uint8 a);

// Call corresponding SDL function.
// If error, print to `stderr` and exit(1)..
void Sdlu_RenderDrawLine(
    SDL_Renderer *const renderer,
    const int x1, const int y1,
    const int x2, const int y2);

// Save .bmp image of given renderer to given path.
// If error, print to `stderr`.
void Sdlu_RendererSaveBmp(
    SDL_Renderer *const renderer, const char *const path);

// Call corresponding SDL function.
// If error, print to `stderr` and exit(1).
void Sdlu_SetRelativeMouseMode(SDL_bool enabled);

// Call corresponding SDL function.
// If error, print to `stderr` and exit(1).
void Sdlu_SetWindowFullscreen(SDL_Window *const w, const Uint32 flags);

// If the window is currently fullscreen w/ either possible flag,
//  remove fullscreen flag.
// If no fullscreen flag present, the given flag is applied.
// If error, print to `stderr` and exit(1).
void Sdlu_ToggleFullscreenFlag(
    SDL_Window *const w, const Uint32 fullscreenFlag);

#ifdef __cplusplus
}
#endif

#endif // SDLU_H
