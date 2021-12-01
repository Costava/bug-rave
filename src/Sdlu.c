#include "Sdlu.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

SDL_Renderer *Sdlu_CreateRenderer(
    SDL_Window *const window,
    const int index,
    const Uint32 flags)
{
    SDL_Renderer *const renderer = SDL_CreateRenderer(window, index, flags);

    if (renderer == NULL) {
        fprintf(stderr, "%s: SDL_CreateRenderer(%p, %d, %d) error: %s\n",
            __func__, window, index, flags, SDL_GetError());
        exit(1);
    }

    return renderer;
}

SDL_Window *Sdlu_CreateWindow(
    const char *const title,
    const int x, const int y,
    const int w, const int h,
    const Uint32 flags)
{
    SDL_Window *const window = SDL_CreateWindow(title, x, y, w, h, flags);

    if (window == NULL) {
        fprintf(stderr, "%s: SDL_CreateWindow(%s, %d, %d, %d, %d, %d) "
            "error: %s\n",
            __func__, title, x, y, w, h, flags, SDL_GetError());
        exit(1);
    }

    return window;
}

void Sdlu_CreateWindowAndRenderer(
    const int width,
    const int height,
    const Uint32 window_flags,
    SDL_Window **window,
    SDL_Renderer **renderer)
{
    const int code = SDL_CreateWindowAndRenderer(
        width, height, window_flags, window, renderer);

    if (code != 0) {
        fprintf(stderr, "%s: SDL_CreateWindowAndRenderer(%d, %d, %d, %p, %p) "
            "error: %s\n",
            __func__, width, height, window_flags, window, renderer,
            SDL_GetError());
        exit(1);
    }
}

void Sdlu_GetDesktopDisplayMode(
    const int displayIndex, SDL_DisplayMode *const mode)
{
    const int code = SDL_GetDesktopDisplayMode(displayIndex, mode);

    if (code != 0) {
        fprintf(stderr, "%s: SDL_GetDesktopDisplayMode(%d, %p) returned %d "
            "instead of 0. Error: %s\n",
            __func__, displayIndex, mode, code, SDL_GetError());
        exit(1);
    }
}

SDL_Renderer *Sdlu_GetRenderer(SDL_Window *const window) {
    SDL_Renderer *const renderer = SDL_GetRenderer(window);

    if (renderer == NULL) {
        fprintf(stderr, "%s: SDL_GetRenderer(%p) error: %s\n",
            __func__, window, SDL_GetError());
        exit(1);
    }

    return renderer;
}

void Sdlu_GetRendererOutputSize(
    SDL_Renderer *const renderer,
    int *const w,
    int *const h)
{
    const int code = SDL_GetRendererOutputSize(renderer, w, h);

    if (code != 0) {
        fprintf(stderr, "%s: SDL_GetRendererOutputSize(%p, %p, %p) returned "
            "%d instead of 0. Error: %s\n",
            __func__, renderer, w, h, code, SDL_GetError());
        exit(1);
    }
}

int Sdlu_GetWindowDisplayIndex(SDL_Window *const window) {
    const int index = SDL_GetWindowDisplayIndex(window);

    if (index < 0) {
        fprintf(stderr, "%s: SDL_GetWindowDisplayIndex(%p) returned: "
            "%d. Error: %s\n", __func__, window, index, SDL_GetError());
        exit(1);
    }

    return index;
}

void Sdlu_Init(Uint32 flags) {
    const int code = SDL_Init(flags);

    if (code != 0) {
        fprintf(stderr, "%s: SDL_Init(%d) returned %d instead of 0. "
            "Error: %s\n", __func__, flags, code, SDL_GetError());
        exit(1);
    }
}

void Sdlu_RenderDrawPoint(
    SDL_Renderer *const renderer, const int x, const int y)
{
    const int code = SDL_RenderDrawPoint(renderer, x, y);

    if (code != 0) {
        fprintf(stderr, "%s: SDL_RenderDrawPoint(%p, %d, %d) returned %d "
            "instead of 0. Error: %s\n",
            __func__, renderer, x, y, code, SDL_GetError());
        exit(1);
    }
}

void Sdlu_RenderFillRect(
    SDL_Renderer *const renderer, const SDL_Rect *const rect)
{
    const int code = SDL_RenderFillRect(renderer, rect);

    if (code != 0) {
        fprintf(stderr, "%s: SDL_RenderFillRect(%p, %p) returned %d "
            "instead of 0. Error: %s\n",
            __func__, renderer, rect, code, SDL_GetError());
        exit(1);
    }
}

void Sdlu_SetRenderDrawColor(
    SDL_Renderer *const renderer,
    const Uint8 r, const Uint8 g, const Uint8 b, const Uint8 a)
{
    const int code = SDL_SetRenderDrawColor(renderer, r, g, b, a);

    if (code != 0) {
        fprintf(stderr, "%s: SDL_SetRenderDrawColor(%p, %d, %d, %d, %d) "
            "returned %d instead of 0. Error: %s\n",
            __func__, renderer, r, g, b, a, code, SDL_GetError());
        exit(1);
    }
}

void Sdlu_RenderDrawLine(
    SDL_Renderer *const renderer,
    const int x1, const int y1,
    const int x2, const int y2)
{
    const int code = SDL_RenderDrawLine(renderer, x1, y1, x2, y2);

    if (code != 0) {
        fprintf(stderr, "%s: SDL_RenderDrawLine(%p, %d, %d, %d, %d) returned "
            "%d instead of 0. Error: %s\n",
            __func__, renderer, x1, y1, x2, y2, code, SDL_GetError());
        exit(1);
    }
}

void Sdlu_RendererSaveBmp(
    SDL_Renderer *const renderer, const char *const path)
{
    int w;
    int h;
    Sdlu_GetRendererOutputSize(renderer, &w, &h);

    // https://wiki.libsdl.org/SDL_CreateRGBSurface
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
    const Uint32 rmask = 0xff000000;
    const Uint32 gmask = 0x00ff0000;
    const Uint32 bmask = 0x0000ff00;
    const Uint32 amask = 0x000000ff;
#else
    const Uint32 rmask = 0x000000ff;
    const Uint32 gmask = 0x0000ff00;
    const Uint32 bmask = 0x00ff0000;
    const Uint32 amask = 0xff000000;
#endif

    SDL_Surface *const surface =
        SDL_CreateRGBSurface(0, w, h, 32, rmask, gmask, bmask, amask);

    if (surface == NULL) {
        fprintf(stderr, "%s: SDL_CreateRGBSurface("
            "%d, %d, %d, %d, %d, %d, %d, %d) error: %s\n",
            __func__, 0, w, h, 32, rmask, gmask, bmask, amask, SDL_GetError());
        return;
    }

    {
        const int rrp_code = SDL_RenderReadPixels(renderer, NULL,
            surface->format->format,
            surface->pixels,
            surface->pitch);

        if (rrp_code != 0) {
            fprintf(stderr, "%s: SDL_RenderReadPixels(%p, %p, %d, %p, %d) "
                "returned %d instead of 0. Error: %s\n",
                __func__, renderer, NULL, surface->format->format,
                surface->pixels, surface->pitch, rrp_code, SDL_GetError());

            goto cleanup;
        }
    }

    {
        const int sbmp_code = SDL_SaveBMP(surface, path);

        if (sbmp_code != 0) {
            fprintf(stderr, "%s: SDL_SaveBMP(%p, %s) returned %d "
                "instead of 0. Error: %s\n",
                __func__, surface, path, sbmp_code, SDL_GetError());

            goto cleanup;
        }
    }

    cleanup:
    SDL_FreeSurface(surface);
}

void Sdlu_SetRelativeMouseMode(SDL_bool enabled) {
    const int code = SDL_SetRelativeMouseMode(enabled);

    if (code != 0) {
        fprintf(stderr, "%s: SDL_SetRelativeMouseMode(%d) returned %d "
            "instead of 0. Error: %s\n",
            __func__, enabled, code, SDL_GetError());
        exit(1);
    }
}

void Sdlu_SetWindowFullscreen(SDL_Window *const w, const Uint32 flags) {
    const int code = SDL_SetWindowFullscreen(w, flags);

    if (code != 0) {
        fprintf(stderr, "%s: SDL_SetWindowFullscreen(%p, %d) returned %d "
            "instead of 0. Error: %s\n",
            __func__, w, flags, code, SDL_GetError());
        exit(1);
    }
}

void Sdlu_ToggleFullscreenFlag(
    SDL_Window *const w, const Uint32 fullscreenFlag)
{
    // Different ways of being fullscreen:
    // 1. SDL_WINDOW_FULLSCREEN flag.
    // 2. SDL_WINDOW_FULLSCREEN_DESKTOP flag.
    // 3. Window manager keyboard shortcut (or other mechanism) sets drawing
    //     size of window to size of screen e.g. Alt+F11 with XFCE which
    //     resizes the window to fullscreen without setting a fullscreen flag
    //     in the eyes of SDL2.

    const Uint32 windowFlags = SDL_GetWindowFlags(w);
    const bool hasAnyFullscreenFlag =
        windowFlags & (SDL_WINDOW_FULLSCREEN | SDL_WINDOW_FULLSCREEN_DESKTOP);

    const Uint32 flag = (hasAnyFullscreenFlag) ? 0 : fullscreenFlag;

    Sdlu_SetWindowFullscreen(w, flag);
}
