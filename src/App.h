#ifndef APP_H
#define APP_H

#include <stdbool.h>
#include <stdint.h>

#include "SDL2/SDL.h"
#include "SDL2/SDL_ttf.h"
#include "SDL2/SDL_mixer.h"

#include "tinyobj_loader_c.h"

#include "emscr.h"
#include "TriInfo.h"
#include "UnitSpherCoord.h"
#include "v2d.h"
#include "v2i.h"
#include "v3d.h"

#ifdef __cplusplus
extern "C" {
#endif

// Text resources.
// Hold rendered surface and texture of text.
typedef struct TextRes {
    SDL_Surface *surface;
    SDL_Texture *texture;
} TextRes;

typedef struct Bug {
    v3d pos;
    bool alive;
    // Azimuthal angle of bug when bug died.
    double deadAzim;
} Bug;

typedef struct Tex {
    v2i size;
    uint8_t *buf;
    size_t bufLen;
} Tex;

typedef struct App {
    v2i renderSizePx;
    size_t numPixels;

    SDL_Window *window;
    SDL_Renderer *renderer;
    SDL_Texture *texture;

    double *depthBuf;

    bool quit;
    #define APP_MODE_MAIN 0
    #define APP_MODE_GAME 1
    int mode;
    int score;
    bool shakeCamera;
    bool showInfoText;
    bool recording;     // Whether saving each frame.
    time_t recordingId; // Used in frame file names so the frames are grouped.
    uint32_t recordingFrameNum;  // Start at 1.

    double bugZoneRadius;
    v3d cameraPos;
    #define APP_NUM_BUGS 290
    Bug bugs[APP_NUM_BUGS];
    v3d carPos;
    UnitSpherCoord carDir;
    v3d carVel;

    int transToGameDurationMs;
    int transToMainDurationMs;

    // true if transitioning between modes.
    bool transMode;
    double transModeStartTimeMs;
    double transModeEndTimeMs;

    double scoreEndTimeMs;

    v3d initialCarPos;
    UnitSpherCoord initialCarDir;
    v3d initialCameraPos;

    UnitSpherCoord lookDir;
    // Unit vectors
    v3d lookRight;
    v3d lookUp;
    v3d lookBack;

    v2d fov;
    v2d invTanHalfFov;// 1 / tan(fov / 2)

    // TTF_Font *ssFont;
    TTF_Font *monoFont;

    #define APP_NUM_SPLATS 4
    Mix_Chunk *splats[APP_NUM_SPLATS];
    int prevSplat;

    Mix_Music *music;

    TextRes infoTRes;
    TextRes scoreTRes;

    uint8_t *buf;
    size_t bufLen;

    uint8_t *bgBuf;
    size_t bgBufLen;

    double oldTimeMs;
    double accumulatedMs;
    double periodMs;
    uint64_t frameCount; // How many frames have been rendered

    // Mix_Chunk *chunk;
    // Mix_Music *music;
    // bool musicPlaying;

    Tex carTex;
    Tex altCarTex;
    Tex *currentCarTex;
    Tex bugTex;
    Tex deadBugTex;
    Tex *currentTex;

    vectriinfo bugVecTriInfo;
    vectriinfo deadBugVecTriInfo;
    vectriinfo carVecTriInfo;

    uint8_t bgr;
    uint8_t bgg;
    uint8_t bgb;
    uint8_t bga;
    double bgtime;
    double bgperiod;
    uint64_t bgperiodcount;
} App;

// Initialize `app`.
void App_Init(App *const app);

// Our "main loop" function. This callback receives the current time as
// reported by the browser, and the user data we provide in the call to
// emscripten_request_animation_frame_loop().
EM_BOOL App_Iterate(double timeMs, void* userData);

// Deinitialize `app`. Clean up internals.
void App_Deinit(App *const app);

#ifdef __cplusplus
}
#endif

#endif // APP_H
