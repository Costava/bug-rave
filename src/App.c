#include "App.h"

#include <float.h>
#include <stdlib.h>
#include <time.h>

#include "stb_image.h"

#define TINYOBJ_LOADER_C_IMPLEMENTATION
#include "tinyobj_loader_c.h"

#include "Clock.h"
#include "Mem.h"
#include "M_PI.h"
#include "Sdlu.h"
#include "TriInfo.h"
#include "UnitSpherCoord.h"
#include "Util.h"
#include "v2d.h"
#include "v3d.h"

#define APP_MUSIC

///////////////////////////////////////////////////////////////////////////////

static double DegToRad(const double deg) {
    return deg * (M_PI / 180.0);
}

static double RadToDeg(const double rad) {
    return rad * (180.0 / M_PI);
}

// For the given point relative to the given camera position,
//  orthographically project onto given look axes.
// Assume the look vectors are unit magnitude.
static v3d OrthoProject(const App *const app, const v3d point) {
    const v3d relativePoint = v3d_Sub(point, app->cameraPos);
    return (v3d) {
        v3d_Dot(relativePoint, app->lookRight),
        v3d_Dot(relativePoint, app->lookUp),
        v3d_Dot(relativePoint, app->lookBack)
    };
}

static v3d OrthoToScreen(const App *const app, const v3d ortho) {
    return (v3d) {
        .x = (app->invTanHalfFov.x * (+ortho.x) / fabs(ortho.z) + 0.5)
            * (app->renderSizePx.x - 1.0),
        .y = (app->invTanHalfFov.y * (-ortho.y) / fabs(ortho.z) + 0.5)
            * (app->renderSizePx.y - 1.0),
        .z = -ortho.z};
}

static v3d WorldToScreen(const App *const app, const v3d point) {
    const v3d ortho = OrthoProject(app, point);
    return OrthoToScreen(app, ortho);
}

static void SetHfov(App *const app, const double hfovRad) {
    const double tanHalfHfov = tan(hfovRad / 2.0);
    app->fov = (v2d) {
        .x = hfovRad,
        .y = 2.0 *
            atan(tanHalfHfov / app->renderSizePx.x * app->renderSizePx.y)};

    app->invTanHalfFov = (v2d) {
        .x = 1.0 / tan(app->fov.x / 2.0),
        .y = 1.0 / tan(app->fov.y / 2.0)};
}

static void AllocateDepthBuf(App *const app) {
    free(app->depthBuf);
    app->depthBuf = Mem_Alloc(
        sizeof(app->depthBuf[0]) * app->numPixels);
}

static void ResetDepthBuf(App *const app) {
    for (size_t i = 0; i < app->numPixels; i += 1) {
        app->depthBuf[i] = DBL_MAX;
    }
}

static void LookAt(App *const app, v3d pos) {
    const v3d forward = v3d_Unit(v3d_Sub(pos, app->cameraPos));

    UnitSpherCoord spher = UnitSpherCoord_FromCartesian(forward);
    app->lookDir = spher;
    spher.incl -= (M_PI / 2.0);

    app->lookUp    = UnitSpherCoord_ToCartesian(spher);
    app->lookBack  = v3d_Invert(forward);
    app->lookRight = v3d_Cross(app->lookUp, app->lookBack);
}

void LookUsingUnitSpherCoord(App *const app, UnitSpherCoord spher) {
    const v3d forward = UnitSpherCoord_ToCartesian(spher);

    app->lookDir = spher;

    spher.incl -= (M_PI / 2.0);

    app->lookUp    = UnitSpherCoord_ToCartesian(spher);
    app->lookBack  = v3d_Invert(forward);
    app->lookRight = v3d_Cross(app->lookUp, app->lookBack);
}



void DrawTriPixel(
    App *const app,
    TriInfo *const triInfo,
    int x, int y, double z,
    double texX,
    double texY)
{
    if (z <= 0.0) {
        // Behind camera.
        return;
    }

    if (texX < 0.0) {
        if (floor(texX) == texX) {
            texX = 1.0;
        }
        else {
            texX += -floor(texX);
        }
    }
    else if (texX > 1.0) {
        if (floor(texX) == texX) {
            texX = 1.0;
        }
        else {
            texX = texX - floor(texX);
        }
    }

    if (texY < 0.0) {
        if (floor(texY) == texY) {
            texY = 1.0;
        }
        else {
            texY += -floor(texY);
        }
    }
    else if (texY > 1.0) {
        if (floor(texY) == texY) {
            texY = 1.0;
        }
        else {
            texY = texY - floor(texY);
        }
    }

    // Both coords now in range [0.0, 1.0]
    // printf("tex: %lf %lf\n", texX, texY);

    v2i texPx = {
        (int)round(texX * (app->currentTex->size.x - 1)),
        (int)round(texY * (app->currentTex->size.y - 1))};
    // printf("texPx: %d %d\n", texPx.x, texPx.y);

    const int dbIndex = y * app->renderSizePx.x + x;

    Uint8 r, g, b, a;
    const int red = (texPx.y * app->currentTex->size.x + texPx.x) * 4;
    r = app->currentTex->buf[red + 0];
    g = app->currentTex->buf[red + 1];
    b = app->currentTex->buf[red + 2];
    a = app->currentTex->buf[red + 3];

    const bool show = (a > 0) ||
    (
        (app->currentTex == &app->carTex)
        ||
        (app->currentTex == &app->altCarTex)
    );

    if (z < app->depthBuf[dbIndex] && show) {
        app->depthBuf[dbIndex] = z;

        // SDL_GetRenderDrawColor(app->renderer, &r, &g, &b, &a);

        // const uint8_t val = (uint8_t)(z / 25.0 * 255.0);
        // r = val;
        // g = val;
        // b = val;
        // a = 255;

        // r = texX * 255.0;
        // g = texY * 255.0;
        // b = 50;
        // a = 255;

        // r = texPx.x % 256;
        // g = texPx.y % 256;
        // b = 100;
        // a = 255;

        size_t i = (size_t)dbIndex * 4;
        app->buf[i] = r; i += 1;
        app->buf[i] = g; i += 1;
        app->buf[i] = b; i += 1;
        app->buf[i] = a; i += 1;
    }
}

// Triangle has a horizontal flat side (same value on y-axis across side).
// [0] in t is not part of flat side
// fill[0] is not part of flat side.
// fill[1] has lesser x coord than fill[2]
void FillHorizFlatTriangle(
    App *const app,
    TriInfo *const t,
    v3d *const fill)
{
    double invslope1;
    double invslope2;
    double curx1;
    double curx2;
    int startY;
    int endY;

    double fullStartZ1;
    double fullEndZ1;
    double fullStartZ2;
    double fullEndZ2;

    double fullStartTexX1;
    double fullEndTexX1;
    double fullStartTexX2;
    double fullEndTexX2;

    double fullStartTexY1;
    double fullEndTexY1;
    double fullStartTexY2;
    double fullEndTexY2;

    v3d startPoint1;
    v3d startPoint2;
    double distance1;
    double distance2;
    if (fill[0].y < fill[1].y) {
        invslope1 = (fill[1].x - fill[0].x) / (fill[1].y - fill[0].y);
        invslope2 = (fill[2].x - fill[0].x) / (fill[2].y - fill[0].y);
        curx1 = fill[0].x;
        curx2 = fill[0].x;
        startY = (int)round(fill[0].y);
        endY   = (int)round(fill[1].y);

        fullStartZ1 = t->set[0].screen.z;
        fullStartZ2 = t->set[0].screen.z;
        fullEndZ1   = t->set[1].screen.z;
        fullEndZ2   = t->set[2].screen.z;

        fullStartTexX1 = t->set[0].tex.x;
        fullStartTexX2 = t->set[0].tex.x;
        fullEndTexX1   = t->set[1].tex.x;
        fullEndTexX2   = t->set[2].tex.x;

        fullStartTexY1 = t->set[0].tex.y;
        fullStartTexY2 = t->set[0].tex.y;
        fullEndTexY1   = t->set[1].tex.y;
        fullEndTexY2   = t->set[2].tex.y;

        startPoint1 = t->set[0].screen;
        startPoint2 = t->set[0].screen;
        distance1 = v3d_Distance2(t->set[0].screen, t->set[1].screen);
        distance2 = v3d_Distance2(t->set[0].screen, t->set[2].screen);
    }
    else {
        invslope1 = (fill[0].x - fill[1].x) / (fill[0].y - fill[1].y);
        invslope2 = (fill[0].x - fill[2].x) / (fill[0].y - fill[2].y);
        curx1 = fill[1].x;
        curx2 = fill[2].x;
        startY = (int)round(fill[1].y);
        endY   = (int)round(fill[0].y);

        fullStartZ1 = t->set[1].screen.z;
        fullStartZ2 = t->set[2].screen.z;
        fullEndZ1   = t->set[0].screen.z;
        fullEndZ2   = t->set[0].screen.z;

        fullStartTexX1 = t->set[1].tex.x;
        fullStartTexX2 = t->set[2].tex.x;
        fullEndTexX1   = t->set[0].tex.x;
        fullEndTexX2   = t->set[0].tex.x;

        fullStartTexY1 = t->set[1].tex.y;
        fullStartTexY2 = t->set[2].tex.y;
        fullEndTexY1   = t->set[0].tex.y;
        fullEndTexY2   = t->set[0].tex.y;

        startPoint1 = t->set[1].screen;
        startPoint2 = t->set[2].screen;
        distance1 = v3d_Distance2(t->set[0].screen, t->set[1].screen);
        distance2 = v3d_Distance2(t->set[0].screen, t->set[2].screen);
    }

    if (startY < 0) {
        curx1 += invslope1 * (-startY);
        curx2 += invslope2 * (-startY);
        startY = 0;
    }

    const int highY = (app->renderSizePx.y - 1);
    if (endY > highY) {
        endY = highY;
    }

    const int highestSeenX = (fill[0].x > fill[2].x)
        ? (int)round(fill[0].x)
        : (int)round(fill[2].x);
    const int screenRight = (app->renderSizePx.x - 1);
    const int maxX = (highestSeenX > screenRight) ? screenRight : highestSeenX;

    const int lowestSeenX = (fill[0].x < fill[1].x)
        ? (int)round(fill[0].x)
        : (int)round(fill[1].x);
    const int minX = (lowestSeenX < 0) ? 0 : lowestSeenX;

    for (int y = startY; y <= endY; y += 1)
    {
        // At y, draw horiz line from curx1 to curx2

        int startX = (int)round(curx1);
        const int fullStartX = startX;
        if (startX < minX) {
            startX = minX;
        }

        int endX = (int)round(curx2);
        const int fullEndX = endX;
        if (endX > maxX) {
            endX = maxX;
        }

        const double yProp1 =
            v3d_Distance2((v3d){fullStartX,y,0.0}, startPoint1) / distance1;

        const double startZ = 1.0 /
            (yProp1 / fullEndZ1 + (1.0 - yProp1) / fullStartZ1);

        // const double startTexX = 1.0 /
        //     (yProp1 / fullEndTexX1 + (1.0 - yProp1) / fullStartTexX1);
        const double startTexX =
            (yProp1 * fullEndTexX1 + (1.0 - yProp1) * fullStartTexX1);

        // const double startTexY = 1.0 /
        //     (yProp1 / fullEndTexY1 + (1.0 - yProp1) / fullStartTexY1);
        const double startTexY =
            (yProp1 * fullEndTexY1 + (1.0 - yProp1) * fullStartTexY1);

        if (startX == endX) {
            DrawTriPixel(
                app,
                t,
                startX, y, startZ,
                startTexX, startTexY
            );
        }
        else {
            const double yProp2 =
                v3d_Distance2((v3d){fullEndX,y,0.0}, startPoint2) / distance2;

            const double endZ = 1.0 /
                (yProp2 / fullEndZ2 + (1.0 - yProp2) / fullStartZ2);

            // const double endTexX = 1.0 /
            //     (yProp2 / fullEndTexX2 + (1.0 - yProp2) / fullStartTexX2);
            const double endTexX =
                (yProp2 * fullEndTexX2 + (1.0 - yProp2) * fullStartTexX2);

            // const double endTexY = 1.0 /
            //     (yProp2 / fullEndTexY2 + (1.0 - yProp2) / fullStartTexY2);
            const double endTexY =
                (yProp2 * fullEndTexY2 + (1.0 - yProp2) * fullStartTexY2);

            for (int x = startX; x <= endX; x += 1) {
                const double a =
                    (x - fullStartX) / (double)(fullEndX - fullStartX);
                DrawTriPixel(
                    app,
                    t,
                    x, y,
                    1.0 / (a / endZ    + (1.0 - a) / startZ),
                    // 1.0 / (a / endTexX + (1.0 - a) / startTexX),
                    (a * endTexX + (1.0 - a) * startTexX),
                    // 1.0 / (a / endTexY + (1.0 - a) / startTexY)
                    (a * endTexY + (1.0 - a) * startTexY)
                );
            }
        }

        curx1 += invslope1;
        curx2 += invslope2;
    }
}

// Assume points in triInfo are sorted ascending by .set.screen.y
void FillTriangle(
    App *const app,
    TriInfo *const t)
{
    // Check for trivial case of greater flat triangle
    if (t->set[1].screen.y == t->set[2].screen.y) {

        if (t->set[2].screen.x < t->set[1].screen.x) {
            TriInfoSet temp = t->set[1];
            t->set[1] = t->set[2];
            t->set[2] = temp;
        }

        FillHorizFlatTriangle(app, t,
            (v3d[]) {
                t->set[0].screen,
                t->set[1].screen,
                t->set[2].screen});
    }
    // Check for trivial case of lesser flat triangle
    else if (t->set[0].screen.y == t->set[1].screen.y) {
        if (t->set[0].screen.x < t->set[1].screen.x) {
            TriInfoSet temp = t->set[0];
            t->set[0] = t->set[2];
            t->set[2] = t->set[1];
            t->set[1] = temp;
        }
        else {
            TriInfoSet temp = t->set[0];
            t->set[0] = t->set[2];
            t->set[2] = temp;
        }

        FillHorizFlatTriangle(app, t,
            (v3d[]) {
                t->set[0].screen,
                t->set[1].screen,
                t->set[2].screen});
    }
    else
    {
        // General case: split the triangle
        const v3d p3 = {
            t->set[0].screen.x
            +
            (
                (t->set[1].screen.y - t->set[0].screen.y)
                /
                (t->set[2].screen.y - t->set[0].screen.y)
            )
            *
            (t->set[2].screen.x - t->set[0].screen.x),
            t->set[1].screen.y,
            -999.0 // This z gets ignored.
        };

        if (t->set[1].screen.x < p3.x) {
            FillHorizFlatTriangle(app, t,
                (v3d[]) {
                    t->set[0].screen,
                    t->set[1].screen,
                    p3});

            TriInfoSet temp = t->set[0];
            t->set[0] = t->set[2];
            t->set[2] = temp;

            FillHorizFlatTriangle(app, t,
                (v3d[]) {
                    t->set[0].screen,
                    t->set[1].screen,
                    p3});
        }
        else {
            TriInfoSet temp = t->set[1];
            t->set[1] = t->set[2];
            t->set[2] = temp;

            FillHorizFlatTriangle(app, t,
                (v3d[]) {
                    t->set[0].screen,
                    p3,
                    t->set[2].screen});

            TriInfoSet temp2 = t->set[0];
            t->set[0] = t->set[1];
            t->set[1] = temp2;

            FillHorizFlatTriangle(app, t,
                (v3d[]) {
                    t->set[0].screen,
                    p3,
                    t->set[2].screen});
        }
    }
}

// Only world and tex are expected to be populated.
static void DrawTriInfo(App *const app, TriInfo *const t) {
    // Populate .screen of sets
    for (int i = 0; i < 3; i += 1) {
        t->set[i].screen = WorldToScreen(app, t->set[i].world);
    }

    // Sort by .screen.y ascending
    for (int i = 0; i < 2; i += 1) {
        for (int j = i + 1; j < 3; j += 1) {
            if (t->set[j].screen.y < t->set[i].screen.y) {
                TriInfoSet temp = t->set[i];
                t->set[i] = t->set[j];
                t->set[j] = temp;
            }
        }
    }

    const bool unseen =
        (t->set[0].screen.z <= 0.0) &&
        (t->set[1].screen.z <= 0.0) &&
        (t->set[2].screen.z <= 0.0);

    if (unseen) {
        return;
    }

    FillTriangle(app, t);
}

static void DrawTri(
    App *const app, v3d v0, v3d v1, v3d v2, const v2d *const tex)
{
    TriInfo triInfo;

    const v3d s0 = WorldToScreen(app, v0);

    triInfo.set[0] = (TriInfoSet){
        .world = v0,
        .screen = s0,
        .tex = tex[0]};

    const v3d s1 = WorldToScreen(app, v1);

    if (s1.y < triInfo.set[0].screen.y) {
        triInfo.set[1] = triInfo.set[0];
        triInfo.set[0] = (TriInfoSet){
            .world = v1,
            .screen = s1,
            .tex = tex[1]};
    }
    else {
        triInfo.set[1] = (TriInfoSet){
            .world = v1,
            .screen = s1,
            .tex = tex[1]};
    }

    const v3d s2 = WorldToScreen(app, v2);

    if (s2.y > triInfo.set[1].screen.y) {
        triInfo.set[2] = (TriInfoSet){
            .world = v2,
            .screen = s2,
            .tex = tex[2]};
    }
    else if (s2.y > triInfo.set[0].screen.y) {
        triInfo.set[2] = triInfo.set[1];
        triInfo.set[1] = (TriInfoSet){
            .world = v2,
            .screen = s2,
            .tex = tex[2]};
    }
    else {
        triInfo.set[2] = triInfo.set[1];
        triInfo.set[1] = triInfo.set[0];
        triInfo.set[0] = (TriInfoSet){
            .world = v2,
            .screen = s2,
            .tex = tex[2]};
    }

    // printf("tex\n");
    // for (int i = 0; i < 3; i += 1) {
    //     printf("coord %lf %lf\n", triInfo.set[i].tex.x, triInfo.set[i].tex.y);
    // }

    const bool unseen =
        (triInfo.set[0].screen.z <= 0.0) &&
        (triInfo.set[1].screen.z <= 0.0) &&
        (triInfo.set[2].screen.z <= 0.0);

    if (unseen) {
        return;
    }

    FillTriangle(app, &triInfo);
}

static void Reset(App *app) {
    app->shakeCamera = true;
    app->score = 0;

    for (int i = 0; i < APP_NUM_BUGS; i += 1) {
        // const int zoneWidth = 4500;
        // app->bugs[i].pos = (v3d) {
        //     .x = rand() % zoneWidth - (zoneWidth * 0.5),
        //     .y = rand() % zoneWidth - (zoneWidth * 0.5),
        //     .z = 0.0};

        app->bugs[i].alive = true;

        const double radius = app->bugZoneRadius;
        const double r = (double)rand() / RAND_MAX * radius;
        const double a = (double)rand() / RAND_MAX * 2.0 * M_PI;
        app->bugs[i].pos = (v3d) {
            .x = radius + r * cos(a),
            .y = radius + r * sin(a),
            .z = 0.0};

        // printf("x y z: %lf %lf %lf\n",
        //     app->bugs[i].pos.x,
        //     app->bugs[i].pos.y,
        //     app->bugs[i].pos.z);
    }

    app->carPos = app->initialCarPos;
    // printf("carPos: %lf %lf %lf\n",
    //     app->carPos.x, app->carPos.y, app->carPos.z);
    app->carDir = app->initialCarDir;
    app->carVel = (v3d) { 0.0, 0.0, 0.0 };

    app->cameraPos = app->initialCameraPos;
    LookAt(app, app->carPos);
    SetHfov(app, M_PI * 0.5);
    // app->lookDir.azim = M_PI * 0.5;
    // app->lookDir.incl = 3.9 * M_PI / 4.0;
}

static void CopyText(
    SDL_Renderer *const renderer,
    TextRes *const tres,
    int dstx,
    int dsty)
{
    SDL_Rect dstrect = {
        .w = tres->surface->w,
        .h = tres->surface->h,
        .x = dstx,
        .y = dsty};
    SDL_RenderCopy(renderer, tres->texture, NULL, &dstrect);
}

void TextRes_Init(TextRes *const tres) {
    tres->surface = NULL;
    tres->texture = NULL;
}

void TextRes_RenderShaded(
    TextRes *const tres,
    SDL_Renderer *const renderer,
    TTF_Font *const font,
    const char *const text,
    SDL_Color fg,
    SDL_Color bg)
{
    SDL_FreeSurface(tres->surface);
    SDL_DestroyTexture(tres->texture);

    tres->surface = TTF_RenderUTF8_Shaded(font, text, fg, bg);

    if (tres->surface == NULL) {
        fprintf(stderr, "%s: Failed to render text (\"%s\"): %s\n",
            __func__, text, TTF_GetError());
        exit(1);
    }

    tres->texture = SDL_CreateTextureFromSurface(renderer, tres->surface);

    if (tres->texture == NULL) {
        fprintf(stderr, "%s: Failed to create texture from text surface: %s\n",
            __func__, TTF_GetError());
        exit(1);
    }
}

static void UpdateInfoTRes(App *const app) {
    char buf[512];
    snprintf(buf, 512,
        " hfov: %.1lfdeg "
        "vfov: %.1lfdeg "
        "cameraPos: (%.1lf, %.1lf, %.1lf) "
        "back (%.2lf, %.2lf, %.2lf)",
        RadToDeg(app->fov.x),
        RadToDeg(app->fov.y),
        app->cameraPos.x,
        app->cameraPos.y,
        app->cameraPos.z,
        app->lookBack.x,
        app->lookBack.y,
        app->lookBack.z);
    TextRes_RenderShaded(
        &app->infoTRes,
        app->renderer,
        app->monoFont,
        buf,
        (SDL_Color) { 255, 255, 255, 255},
        (SDL_Color) { 0, 0, 0, 255});
}

static void UpdateScoreTRes(App *const app, double timeMs) {
    double secondsLeft = (app->scoreEndTimeMs - timeMs) / 1000.0;
    if (secondsLeft < 0.0) {
        secondsLeft = 0.0;
    }

    char buf[256];
    snprintf(buf, 256,
        " Score: %d   Time Left: %d ", app->score, (int)ceil(secondsLeft));
    TextRes_RenderShaded(
        &app->scoreTRes,
        app->renderer,
        app->monoFont,
        buf,
        (SDL_Color) { 255, 255, 255, 255},
        (SDL_Color) { 0, 0, 0, 255});
}

static void get_file_data(void* ctx, const char* filename, const int is_mtl,
                          const char* obj_filename, char** data, size_t* len)
{
    FILE *const f = fopen(obj_filename, "rb");
    if (f == NULL) {
        fprintf(stderr, "fopen err\n");
        exit(1);
    }

    long int oldTell = ftell(f);
    fseek(f, 0, SEEK_END);
    long int newTell = ftell(f);
    *len = (size_t)(newTell - oldTell);

    // printf("len: %ld\n", *len);

    // fpos_t pos;
    // fgetpos(f, &pos);
    // *len = (size_t)pos;

    fseek(f, 0, SEEK_SET);

    *data = Mem_Alloc(*len);
    size_t numRead = fread(*data, 1, *len, f);

    if (numRead != *len) {
        fprintf(stderr, "len err\n");
        exit(1);
    }

    // printf("pos: %d\n", pos);
    // exit(1);

    fclose(f);
}

static void LoadTex(Tex *const tex, char *const filePath) {
    {
        int n;
        tex->buf = stbi_load(
            filePath,
            &tex->size.x, &tex->size.y, &n, 4);
    }

    if (tex->buf == NULL) {
        fprintf(stderr, "%s: Load %s error\n", __func__, filePath);
        exit(1);
    }

    tex->bufLen =
        (size_t)tex->size.x * (size_t)tex->size.y * 4;
}

void StartMain(App *const app) {
    app->mode = APP_MODE_MAIN;
    Sdlu_SetRelativeMouseMode(SDL_FALSE);

    // Prevent a freeze when we FreeMusic.
    Mix_HaltMusic();

    // SDL_mixer has a bug?
    // Fade-in does not work unless we reload
    // the music before each fade-in.
    if (app->music != NULL) {
        Mix_FreeMusic(app->music);
    }

    app->music =
        Mix_LoadMUS("./assets/Android128 - Rave Try (Blueeskies).ogg");
    if (app->music == NULL) {
        fprintf(stderr, "LoadMUS errored\n");
    }

    app->transMode = false;

    {
        size_t i = 0;
        for (size_t p = 0; p < app->numPixels; p += 1) {
            app->bgBuf[i] =   0; i += 1;
            app->bgBuf[i] =   0; i += 1;
            app->bgBuf[i] =   0; i += 1;
            app->bgBuf[i] = 255; i += 1;
        }
    }

    app->transMode = false;

    app->carPos = app->initialCarPos;
    app->carDir = app->initialCarDir;

    app->cameraPos = (v3d) {
        1.0, -10.0, 10.0};
}

void StartGame(App *const app) {
    app->mode = APP_MODE_GAME;
    Sdlu_SetRelativeMouseMode(SDL_TRUE);

    // #ifdef APP_MUSIC
    // Mix_PlayMusic(app->music, -1);
    // Mix_SetMusicPosition(21.5);
    // #endif

    app->transMode = false;
    app->frameCount = 0;

    Reset(app);
}

void App_Init(App *const app) {
    // printf("%s\n", __func__);

    Sdlu_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO);

    {
        const time_t seed = Util_TimeOrExit(NULL);
        fprintf(stdout, "seed: %ld\n", seed);
        srand((unsigned)seed);
    }

    app->renderSizePx = (v2i) {
        .x = 800,
        .y = 600};
    app->numPixels = (size_t)app->renderSizePx.x * (size_t)app->renderSizePx.y;

    app->window = Sdlu_CreateWindow(
        "Bug Rave",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        app->renderSizePx.x, app->renderSizePx.y,
        0);

    app->renderer = Sdlu_CreateRenderer(
        app->window, -1, SDL_RENDERER_ACCELERATED);

    app->texture = SDL_CreateTexture(
        app->renderer,
        SDL_PIXELFORMAT_ABGR8888,
        SDL_TEXTUREACCESS_STREAMING,
        app->renderSizePx.x, app->renderSizePx.y);

    if (app->texture == NULL) {
        fprintf(stderr, "%s: Failed to create texture: %s\n",
            __func__, SDL_GetError());
        exit(1);
    }

    app->depthBuf = NULL;
    AllocateDepthBuf(app);

    app->quit = false;
    app->mode = APP_MODE_MAIN;
    app->showInfoText = false;
    app->recording = false;

    // app->bgperiod = 416.67;// 144
    app->bgperiod = 333.333;// 180
    app->bgtime = app->bgperiod / 2.0;
    app->bgperiodcount = 0;

    app->bugZoneRadius = 2500.0;

    app->initialCarPos =
        (v3d) { app->bugZoneRadius, -app->bugZoneRadius / 2.0, 0.0 };

    app->initialCarDir = (UnitSpherCoord) {
        .azim = M_PI * 0.5,
        .incl = M_PI * 0.5,
    };

    app->initialCameraPos = app->initialCarPos;
    app->initialCameraPos.y -= 100.0;
    app->initialCameraPos.z += 50.0;

    app->transToGameDurationMs = 4000;
    app->transToMainDurationMs = 1000;

    if (TTF_Init() != 0) {
        fprintf(stderr, "TTF_Init error: %s\n", TTF_GetError());
        exit(1);
    }

    // {
    //     const char ssFontPath[] = "./assets/OpenSans-Regular.ttf";
    //     app->ssFont = TTF_OpenFont(ssFontPath, 16);
    //
    //     if (app->ssFont == NULL) {
    //         fprintf(stderr, "Failed to open font from %s because: %s\n",
    //             ssFontPath, TTF_GetError());
    //         exit(1);
    //     }
    // }

    {
        const char monoFontPath[] = "./assets/NotoSansMono-Regular.ttf";
        app->monoFont = TTF_OpenFont(monoFontPath, 12);

        if (app->monoFont == NULL) {
            fprintf(stderr, "Failed to open font from %s because: %s\n",
                monoFontPath, TTF_GetError());
            exit(1);
        }
    }

    Mix_Init(0);
    if (Mix_OpenAudio(
        MIX_DEFAULT_FREQUENCY, MIX_DEFAULT_FORMAT, 2, 4096) != 0)
    {
        fprintf(stderr, "OpenAudio errored\n");
    }

    const int splatVol = 50;

    app->splats[0] = Mix_LoadWAV("./assets/splat0.wav");
    if (app->splats[0] == NULL) {
        fprintf(stderr, "LoadWAV errored\n");
    }
    else {
        Mix_VolumeChunk(app->splats[0], splatVol);
    }
    app->splats[1] = Mix_LoadWAV("./assets/splat1.wav");
    if (app->splats[1] == NULL) {
        fprintf(stderr, "LoadWAV errored\n");
    }
    else {
        Mix_VolumeChunk(app->splats[1], splatVol);
    }
    app->splats[2] = Mix_LoadWAV("./assets/splat2.wav");
    if (app->splats[2] == NULL) {
        fprintf(stderr, "LoadWAV errored\n");
    }
    else {
        Mix_VolumeChunk(app->splats[2], splatVol);
    }
    app->splats[3] = Mix_LoadWAV("./assets/splat3.wav");
    if (app->splats[3] == NULL) {
        fprintf(stderr, "LoadWAV errored\n");
    }
    else {
        Mix_VolumeChunk(app->splats[3], splatVol);
    }

    app->prevSplat = -1;

    // app->music =
    //     Mix_LoadMUS("./assets/Android128 - Rave Try (Blueeskies).ogg");
    // if (app->music == NULL) {
    //     fprintf(stderr, "LoadMUS errored\n");
    // }

    app->music = NULL;

    // app->musicPlaying = false;

    TextRes_Init(&app->infoTRes);
    UpdateInfoTRes(app);

    TextRes_Init(&app->scoreTRes);

    app->bufLen = app->numPixels * 4;
    app->buf = Mem_Alloc(sizeof(app->buf[0]) * app->bufLen);

    app->bgBufLen = app->numPixels * 4;
    app->bgBuf = Mem_Alloc(sizeof(app->bgBuf[0]) * app->bgBufLen);

    {
        size_t i = 0;
        for (size_t p = 0; p < app->numPixels; p += 1) {
            app->buf[i] =  50; i += 1;
            app->buf[i] = 105; i += 1;
            app->buf[i] =  50; i += 1;
            app->buf[i] = 255; i += 1;
        }
    }

    #ifdef __EMSCRIPTEN__
    app->oldTimeMs = 0.0;
    #else
    app->oldTimeMs = Clock_GetTimeMs();
    #endif

    app->accumulatedMs = 0.0;
    app->periodMs = 1000.0 / 60.0;
    app->frameCount = 0;

    LoadTex(&app->carTex, "./assets/carTex.png");
    LoadTex(&app->bugTex, "./assets/grasshopper.png");
    LoadTex(&app->deadBugTex, "./assets/deadGrasshopper.png");

    LoadTex(&app->altCarTex, "./assets/carTex.png");
    {
        // Change it.
        for (size_t i = 0; i < app->altCarTex.bufLen; i += 2) {
            app->altCarTex.buf[i] = 255;
        }
    }

    app->currentCarTex = &app->carTex;

    ///////////////////////////////////////////////////////////////////////////

    tinyobj_attrib_t attrib;
    tinyobj_shape_t* shapes = NULL;
    size_t num_shapes;
    tinyobj_material_t* materials = NULL;
    size_t num_materials;

    unsigned int flags = TINYOBJ_FLAG_TRIANGULATE;
    int ret =
        tinyobj_parse_obj(&attrib, &shapes, &num_shapes, &materials,
            &num_materials,
            // "./assets/all_pos_cube_from_quads.obj",
            "./assets/car.obj",
            get_file_data, NULL, flags);
    if (ret != TINYOBJ_SUCCESS) {
        fprintf(stderr, "tinyobj error\n");
        exit(1);
    }

    // printf("# of shapes    = %d\n", (int)num_shapes);
    // printf("# of materials = %d\n", (int)num_materials);
    // printf("num_vertices: %d\n", attrib.num_vertices);
    // printf("num_faces: %d\n", attrib.num_faces);
    // printf("num_face_num_verts: %d\n", attrib.num_face_num_verts);
    // printf("face_num_verts[0]: %d\n", attrib.face_num_verts[0]);

    const double bugHalfWidth = 60.0;

    vectriinfo_init(&app->bugVecTriInfo, 2, VEC_GROW_MODE_ADD, 2);

    {
        const double halfWidth = bugHalfWidth;
        const double halfHeight = halfWidth
            / app->bugTex.size.x * app->bugTex.size.y;
        const double heightOffset = 19.0;

        TriInfo newTriInfo;

        newTriInfo.set[0] = (TriInfoSet) {
            .world = (v3d) {
                0, -halfWidth, -halfHeight + heightOffset
            },
            .tex = (v2d) {0.0, 1.0}
        };
        newTriInfo.set[1] = (TriInfoSet) {
            .world = (v3d) {
                0, -halfWidth, +halfHeight + heightOffset
            },
            .tex = (v2d) {0.0, 0.0}
        };
        newTriInfo.set[2] = (TriInfoSet) {
            .world = (v3d) {
                0, +halfWidth, -halfHeight + heightOffset
            },
            .tex = (v2d) {1.0, 1.0}
        };
        vectriinfo_push_back(&app->bugVecTriInfo, newTriInfo);

        newTriInfo.set[0] = (TriInfoSet) {
            .world = (v3d) {
                0, -halfWidth, +halfHeight + heightOffset
            },
            .tex = (v2d) {0.0, 0.0}
        };
        newTriInfo.set[1] = (TriInfoSet) {
            .world = (v3d) {
                0, +halfWidth, +halfHeight + heightOffset
            },
            .tex = (v2d) {1.0, 0.0}
        };
        newTriInfo.set[2] = (TriInfoSet) {
            .world = (v3d) {
                0, +halfWidth, -halfHeight + heightOffset
            },
            .tex = (v2d) {1.0, 1.0}
        };
        vectriinfo_push_back(&app->bugVecTriInfo, newTriInfo);
    }

    vectriinfo_init(&app->deadBugVecTriInfo, 2, VEC_GROW_MODE_ADD, 2);

    {
        const double halfWidth = bugHalfWidth;
        const double halfHeight = halfWidth
            / app->deadBugTex.size.x * app->deadBugTex.size.y;
        const double heightOffset = -14.0;

        TriInfo newTriInfo;

        newTriInfo.set[0] = (TriInfoSet) {
            .world = (v3d) {
                -halfHeight, -halfWidth, heightOffset
            },
            .tex = (v2d) {0.0, 0.0}
        };
        newTriInfo.set[1] = (TriInfoSet) {
            .world = (v3d) {
                halfHeight, halfWidth, heightOffset
            },
            .tex = (v2d) {1.0, 1.0}
        };
        newTriInfo.set[2] = (TriInfoSet) {
            .world = (v3d) {
                halfHeight, -halfWidth, heightOffset
            },
            .tex = (v2d) {0.0, 1.0}
        };
        vectriinfo_push_back(&app->deadBugVecTriInfo, newTriInfo);

        newTriInfo.set[0] = (TriInfoSet) {
            .world = (v3d) {
                -halfHeight, -halfWidth, heightOffset
            },
            .tex = (v2d) {0.0, 0.0}
        };
        newTriInfo.set[1] = (TriInfoSet) {
            .world = (v3d) {
                -halfHeight, halfWidth, heightOffset
            },
            .tex = (v2d) {1.0, 0.0}
        };
        newTriInfo.set[2] = (TriInfoSet) {
            .world = (v3d) {
                halfHeight, halfWidth, heightOffset
            },
            .tex = (v2d) {1.0, 1.0}
        };
        vectriinfo_push_back(&app->deadBugVecTriInfo, newTriInfo);
    }

    vectriinfo_init(&app->carVecTriInfo, attrib.num_faces,
        VEC_GROW_MODE_ADD, 8);

    // printf("capacity: %ld\n", app->carVecTriInfo.capacity);

    /*
    {
      int i;
      for (i = 0; i < num_shapes; i++) {
        printf("shape[%d] name = %s\n", i, shapes[i].name);
      }
    }
    */

    size_t face_offset = 0;
    for (size_t i = 0; i < attrib.num_face_num_verts; i++) {
        size_t f;
        assert(attrib.face_num_verts[i] % 3 ==
             0); /* assume all triangle faces. */
        for (f = 0; f < (size_t)attrib.face_num_verts[i] / 3; f++) {
            size_t k;
            double v[3][3];
            double t[3][2];

            tinyobj_vertex_index_t idx0 =
                attrib.faces[face_offset + 3 * f + 0];
            tinyobj_vertex_index_t idx1 =
                attrib.faces[face_offset + 3 * f + 1];
            tinyobj_vertex_index_t idx2 =
                attrib.faces[face_offset + 3 * f + 2];

            for (k = 0; k < 3; k++) {
                int f0 = idx0.v_idx;
                int f1 = idx1.v_idx;
                int f2 = idx2.v_idx;
                assert(f0 >= 0);
                assert(f1 >= 0);
                assert(f2 >= 0);

                v[0][k] = (double)attrib.vertices[3 * (size_t)f0 + k];
                v[1][k] = (double)attrib.vertices[3 * (size_t)f1 + k];
                v[2][k] = (double)attrib.vertices[3 * (size_t)f2 + k];
            }

            for (k = 0; k < 2; k++) {
                int f0 = idx0.vt_idx;
                int f1 = idx1.vt_idx;
                int f2 = idx2.vt_idx;
                assert(f0 >= 0);
                assert(f1 >= 0);
                assert(f2 >= 0);

                t[0][k] = (double)attrib.texcoords[2 * (size_t)f0 + k];
                t[1][k] = (double)attrib.texcoords[2 * (size_t)f1 + k];
                t[2][k] = (double)attrib.texcoords[2 * (size_t)f2 + k];
            }

            v3d vert[3];
            v2d tex[3];
            for (k = 0; k < 3; k++) {
                const double s = 15.0;
                vert[k].x = v[k][0] * s;
                vert[k].y = v[k][1] * s;
                vert[k].z = v[k][2] * s;

                tex[k].x = t[k][0];
                // Flip y.
                // I guess I need to modify my Blender export settings.
                tex[k].y = 1.0 - t[k][1];

                // printf("vert: %lf %lf %lf\n",
                //     vert[k].x, vert[k].y, vert[k].z);
                // printf("tex: %lf %lf\n", tex[k].x, tex[k].y);
            }

            TriInfo newTriInfo;
            newTriInfo.set[0] = (TriInfoSet) {
                .world = vert[0],
                .tex = tex[0]
            };
            newTriInfo.set[1] = (TriInfoSet) {
                .world = vert[1],
                .tex = tex[1]
            };
            newTriInfo.set[2] = (TriInfoSet) {
                .world = vert[2],
                .tex = tex[2]
            };
            vectriinfo_push_back(&app->carVecTriInfo, newTriInfo);
        }

        face_offset += (size_t)attrib.face_num_verts[i];
    }

    ///////////////////////////////////////////////////////////////////////////

    // Reset(app);
    StartMain(app);
}

typedef struct RelativeMouseMotion {
    Sint32 xrel;
    Sint32 yrel;
} RelativeMouseMotion;

typedef struct MaybeRelativeMouseMotion {
    bool hasValue;
    RelativeMouseMotion value;
} MaybeRelativeMouseMotion;

static void PollEvents(
    App *const app, const double timeMs,const double deltaMs)
{
    // If multiple mousemotion events happen during one frame,
    //  we sum the relative motion and act once on the sum.
    MaybeRelativeMouseMotion motion = { .hasValue = false };
    uint32_t numMouseMotionEvents = 0;

    // Note: This line only removes 'unused' warning.
    numMouseMotionEvents += (0 * (uint32_t)deltaMs);

    SDL_Event event;
    while (SDL_PollEvent(&event) != 0) {
        switch (event.type) {
            case SDL_QUIT:
            {
                app->quit = true;
                break;
            }
            case SDL_WINDOWEVENT: {
                switch (event.window.event) {
                    case SDL_WINDOWEVENT_SIZE_CHANGED:
                    {
                        break;
                    }
                    case SDL_WINDOWEVENT_FOCUS_GAINED:
                    {
                        Sdlu_SetRelativeMouseMode(SDL_TRUE);
                        break;
                    }
                    case SDL_WINDOWEVENT_FOCUS_LOST:
                    {
                        Sdlu_SetRelativeMouseMode(SDL_FALSE);
                        break;
                    }
                }

                break;
            }
            case SDL_MOUSEMOTION:
            {
                if (!motion.hasValue) {
                    motion = (MaybeRelativeMouseMotion) {
                        .hasValue = true,
                        .value = {
                            .xrel = event.motion.xrel,
                            .yrel = event.motion.yrel
                        }
                    };
                }
                else {
                    motion.value.xrel += event.motion.xrel;
                    motion.value.yrel += event.motion.yrel;
                }

                // Assume it never overflows
                // (safe to assume and of low consequence)
                numMouseMotionEvents += 1;

                // int x, y;
                // SDL_GetMouseState(&x, &y);
                // app->tX = x - (app->tWidth / 2);

                break;
            }
            case SDL_MOUSEWHEEL:
            {
                const double newHfov = app->fov.x - 0.1 * event.wheel.y;
                // Bounds check assumes square or landscape aspect ratio.
                if (newHfov > 0.0 && newHfov < M_PI) {
                    SetHfov(app, newHfov);
                }

                break;
            }
            case SDL_MOUSEBUTTONDOWN:
            {
                if (!app->transMode) {
                    app->transMode = true;
                    app->transModeStartTimeMs = timeMs;
                    app->transModeEndTimeMs = app->transModeStartTimeMs +
                        app->transToMainDurationMs;

                    Mix_FadeOutMusic(app->transToMainDurationMs);
                }
                break;
            }
            case SDL_KEYDOWN:
            {
                const SDL_Keycode keycode = event.key.keysym.sym;
                // const SDL_Keymod modState = SDL_GetModState();

                switch (keycode) {
                    case SDLK_F1:
                    {
                        app->showInfoText = !app->showInfoText;
                        break;
                    }
                }
                break;
            }
            case SDL_KEYUP:
            {
                const SDL_Keycode keycode = event.key.keysym.sym;
                // const SDL_Keymod modState = SDL_GetModState();

                switch (keycode) {
                    case SDLK_F11:
                    {
                        break;
                    }
                    case SDLK_r:
                    {
                        // Reset(app);

                        // if (!app->musicPlaying) {
                        //     #ifndef APP_NO_MUSIC
                        //     Mix_PlayMusic(app->music, -1);
                        //     #endif
                        //     app->musicPlaying = true;
                        // }
                        break;
                    }
                    // case SDLK_r:
                    // {
                    //     if (!app->recording) {
                    //         // Reset frame number.
                    //         app->recordingFrameNum = 1;
                    //
                    //         // Get new recordingId.
                    //
                    //         struct timespec ts;
                    //         if (timespec_get(&ts, TIME_UTC) == 0) {
                    //             fprintf(stderr, "FAILED TO START RECORDING. "
                    //                 " timespec_get error\n");
                    //             break;
                    //         }
                    //
                    //         app->recordingId = ts.tv_sec;
                    //
                    //         fprintf(stdout, "Starting recording.\n");
                    //     }
                    //     else {
                    //         fprintf(stdout, "Stopping recording.\n");
                    //     }
                    //
                    //     // Toggle recording.
                    //     app->recording = !app->recording;
                    //     break;
                    // }
                }

                break;
            }
        }
    }

    // // While recording, multiple may happen during one frame.
    // if (numMouseMotionEvents > 1) {
    //     fprintf(stdout, "numMouseMotionEvents: %d\n", numMouseMotionEvents);
    // }

    if (motion.hasValue) {
        const double mouseSens = 0.1e-3;

        app->lookDir.azim -=
            mouseSens * motion.value.xrel * deltaMs;
        app->lookDir.incl +=
            mouseSens * motion.value.yrel * deltaMs;

        // Normalize horizLookRads to range (-2pi, 2pi)
        app->lookDir.azim = fmod(app->lookDir.azim, 2.0 * M_PI);
        // Normalize horizLookRads to range [-pi, pi]
        if (app->lookDir.azim < -M_PI)
            { app->lookDir.azim += 2.0 * M_PI; }
        else if (app->lookDir.azim > M_PI)
            { app->lookDir.azim -= 2.0 * M_PI; }

        // Clamp vertLookRads
        const double minVertLookRads = 0.00001;
        const double maxVertLookRads = M_PI - minVertLookRads;
        if (app->lookDir.incl < minVertLookRads) {
            app->lookDir.incl = minVertLookRads;
        }
        else if (app->lookDir.incl > maxVertLookRads) {
            app->lookDir.incl = maxVertLookRads;
        }
    }
}

EM_BOOL App_IterateMain(double timeMs, void* userData) {
    App *const app = userData;

    SDL_Event event;
    while (SDL_PollEvent(&event) != 0) {
        switch (event.type) {
            case SDL_QUIT:
            {
                app->quit = true;
                break;
            }
            case SDL_WINDOWEVENT: {
                switch (event.window.event) {
                    case SDL_WINDOWEVENT_SIZE_CHANGED:
                    {
                        break;
                    }
                }

                break;
            }
            case SDL_MOUSEMOTION:
            {
                break;
            }
            case SDL_MOUSEWHEEL:
            {
                break;
            }
            case SDL_MOUSEBUTTONDOWN:
            {
                if (!app->transMode) {
                    app->transMode = true;
                    app->transModeStartTimeMs = timeMs;
                    app->transModeEndTimeMs = app->transModeStartTimeMs +
                        app->transToGameDurationMs;

                    Mix_FadeInMusicPos(app->music, -1,
                        app->transToGameDurationMs,
                        21.5 - (app->transToGameDurationMs / 1000.0));
                }
                break;
            }
            case SDL_KEYDOWN:
            {
                const SDL_Keycode keycode = event.key.keysym.sym;
                // const SDL_Keymod modState = SDL_GetModState();

                switch (keycode) {
                    case SDLK_F1:
                    {
                        break;
                    }
                }
                break;
            }
            case SDL_KEYUP:
            {
                const SDL_Keycode keycode = event.key.keysym.sym;
                // const SDL_Keymod modState = SDL_GetModState();

                switch (keycode) {
                    case SDLK_F11:
                    {
                        break;
                    }
                }
                break;
            }
        }
    }

    // Progress towards transitioning from main menu to game.
    double prog;
    if (app->transMode) {
        prog = (timeMs - app->transModeStartTimeMs) /
            (app->transModeEndTimeMs - app->transModeStartTimeMs);

        if (prog > 1.0) {
            prog = 1.0;
        }
    }
    else {
        prog = 0.0;
    }

    double maxT = 8000.0;
    double camX = sin(fmod(timeMs, maxT) / maxT * 2.0 * M_PI) * -400.0 + 120.0;
    double camZ = sin(fmod(timeMs, maxT) / maxT * 2.0 * M_PI) * 50.0 + 15.0;

    v3d nonTransCameraPos = (v3d) {
        camX + app->carPos.x,
        app->carPos.y + 150.0,
        camZ + app->carPos.z
    };

    double hfov = prog * DegToRad(130.0) + (1.0 - prog) * (M_PI * 0.5);

    SetHfov(app, hfov);
    app->carPos = app->initialCarPos;
    app->carDir = app->initialCarDir;

    const v3d nonTransTarget = v3d_Add(app->carPos, (v3d) {45.0, 0.0, 0.0});
    const v3d target = v3d_Lerp(prog, nonTransTarget, app->carPos);

    app->cameraPos = v3d_Lerp(prog,
        nonTransCameraPos, app->initialCameraPos);

    LookAt(app, target);

    SDL_RenderClear(app->renderer);

    for (size_t i = 0; i < app->bufLen; i += 1) {
        app->buf[i] = app->bgBuf[i];
    }

    ResetDepthBuf(app);

    {
        const v3d keep = app->cameraPos;
        const UnitSpherCoord keepDir = app->lookDir;

        app->currentTex = &app->carTex;

        app->cameraPos = v3d_Sub(app->cameraPos, app->carPos);

        const double rad = app->carDir.azim;
        app->lookDir.azim -= rad;
        LookUsingUnitSpherCoord(app, app->lookDir);
        app->cameraPos = v3d_RotateZ(
            app->cameraPos, -rad);

        for (size_t i = 0; i < app->carVecTriInfo.length; i += 1) {
            DrawTriInfo(app, &app->carVecTriInfo.buf[i]);
        }

        LookUsingUnitSpherCoord(app, keepDir);
        app->cameraPos = keep;
    }

    SDL_UpdateTexture(app->texture, NULL, app->buf, app->renderSizePx.x * 4);
    SDL_RenderCopy(app->renderer, app->texture, NULL, NULL);
    SDL_RenderPresent(app->renderer);

    if (prog == 1.0) {
        StartGame(app);
    }

    return EM_TRUE;
}

EM_BOOL App_IterateGame(double timeMs, void* userData) {
    // printf("timeMs: %lf\n", timeMs);

    App *const app = userData;

    const double addMs = timeMs - app->oldTimeMs;
    app->accumulatedMs += addMs;

    app->bgtime += addMs;

    // printf("FPS: %lf\n", 1000.0 / addMs);
    // printf("accumulatedMs: %lf\n", app->accumulatedMs);

    if (app->accumulatedMs < app->periodMs) {
        const double delayMs = floor(app->periodMs - app->accumulatedMs);
        // printf("dead time ms: %lf\n", delayMs);
        SDL_Delay((Uint32)delayMs);
        goto end_of_while_loop;
    }
    else {
        app->accumulatedMs -= app->periodMs;
        app->frameCount += 1;

        if (app->frameCount == 1) {
            app->scoreEndTimeMs = timeMs + 30000.0;
        }
    }

    const double deltaMs = app->periodMs;

    double prog;
    if (app->transMode) {
        prog = (timeMs - app->transModeStartTimeMs) /
            (app->transModeEndTimeMs - app->transModeStartTimeMs);

        if (prog > 1.0) {
            prog = 1.0;
        }
    }
    else {
        prog = 0.0;
    }

    // const double x = fmod(timeMs, 500.0) / 500.0;
    // // printf("x: %lf\n", x);
    // const double s = 1.0 - pow(2.0 * x - 1.0, 1.0 / 5.1);
    // // printf("s: %lf\n", s);
    // const double hfov = M_PI / 2.0 + s * DegToRad(10.0);
    // SetHfov(app, hfov);

    if (app->bgtime > app->bgperiod) {
        app->bgtime = 0;
        app->bgperiodcount += 1;
        int rnd = rand();
        app->bgr = (uint8_t)(rnd >> 0);
        app->bgg = (uint8_t)(rnd >> 8);
        app->bgb = (uint8_t)(rnd >> 16);
        app->bga = 255;

        size_t i = 0;

        // int x = -1;
        // int y = 0;
        // int d[5];// Consistent random values for 1 whole screen.
        // for (int i = 0; i < 5; i += 1) {
        //     d[i] = rand();
        // }
        for (size_t p = 0; p < app->numPixels; p += 1) {
            app->bgBuf[i] = app->bgr; i += 1;
            app->bgBuf[i] = app->bgg; i += 1;
            app->bgBuf[i] = app->bgb; i += 1;
            app->bgBuf[i] = app->bga; i += 1;

            // x += 1;
            // if (x == app->renderSizePx.x) {
            //     x = 0;
            //     y += 1;
            // }
            // double r, g, b;
            //
            // uint64_t cycle = app->bgperiodcount % 3;
            // // cycle = 1;
            //
            // // printf("cycle: %d\n", cycle);
            //
            // switch (cycle) {
            //     case 0:
            //     {
            //         r = x;
            //         g = y;
            //         b = x + y;
            //         break;
            //     }
            //     case 1:
            //     {
            //         r = x * x * 0.5;
            //         g = y * y * 0.5;
            //         b = x + y;
            //         break;
            //     }
            //     case 2:
            //     {
            //         r = y;
            //         g = x + y;
            //         b = x;
            //         break;
            //     }
            //     default:
            //     {
            //         double sd = (d[0] % 200) + 300.0;
            //         double cd = (d[1] % 200) + 300.0;
            //         r = 300.0 * sin(x / sd) + 128.0;
            //         g = 381.0 * cos(y / cd) + 128.0;
            //         b = x + y + cos(x + y) * 100.0;
            //         break;
            //     }
            // }
            //
            // app->bgBuf[i + 0] = (uint8_t)(((int)floor(r)) % 128) + app->bgr/2;
            // app->bgBuf[i + 1] = (uint8_t)(((int)floor(g)) % 128) + app->bgg/2;
            // app->bgBuf[i + 2] = (uint8_t)(((int)floor(b)) % 128) + app->bgb/2;
            // app->bgBuf[i + 3] = app->bga;
            //
            // i += 4;
        }
    }

    // if (app->frameCount & 0b1) {
    //     app->currentCarTex = &app->altCarTex;
    // }
    // else {
    //     app->currentCarTex = &app->carTex;
    // }

    // Change FOV on interval.
    if (app->shakeCamera) {
        const double period = app->bgperiod;
        const double x = fmod(timeMs, period) / period;

        const double y =
            0.5 * cos(M_PI * pow(fabs(2.0 * x - 1.0), 1.0 / 2.6)) + 0.5;

        const double hfov = DegToRad(130.0) - y * DegToRad(39.0);
        SetHfov(app, hfov);
    }

    PollEvents(app, timeMs, deltaMs);

    if ((app->frameCount & 0b111) == 7) {
        UpdateInfoTRes(app);
    }

    const uint8_t *const kbState = SDL_GetKeyboardState(NULL);
    const SDL_Keymod modState = SDL_GetModState();

    if (modState & KMOD_SHIFT) {
        app->currentCarTex = &app->altCarTex;
    }
    else {
        app->currentCarTex = &app->carTex;
    }

    const v3d carForward = UnitSpherCoord_ToCartesian(app->carDir);

    const bool isMoving = v3d_Mag(app->carVel) > 1.99;
    const bool isMovingForward =
        v3d_Dot(carForward, v3d_Unit(app->carVel)) > 0.0;
    const bool wantToTurn =
        (kbState[SDL_SCANCODE_A] == 1) ^ (kbState[SDL_SCANCODE_D] == 1);

    double turnAmountRad = (modState & KMOD_SHIFT) ? 0.14 : 0.07;
    if (wantToTurn && isMoving) {
        if (kbState[SDL_SCANCODE_D] == 1) {
            turnAmountRad = 0 - turnAmountRad;
        }
        if (!isMovingForward) {
            turnAmountRad = 0 - turnAmountRad;
        }
        const double speed = v3d_Mag(app->carVel);
        // printf("speed: %lf\n", speed);
        UnitSpherCoord dir = UnitSpherCoord_FromCartesian(app->carVel);
        dir.azim += turnAmountRad;
        const double transfer = 0.7;
        app->carVel = v3d_Add(
            v3d_Mul(UnitSpherCoord_ToCartesian(dir),
                speed * transfer),
            v3d_Mul(v3d_Unit(app->carVel),
                speed * (1.0 - transfer))
        );

        app->carDir.azim += turnAmountRad;
        // app->lookDir.azim += turnAmountRad;
    }

    const double accelAmount = (modState & KMOD_SHIFT) ? 1.2 : 0.5;
    const v3d accelForward = v3d_Mul(carForward, accelAmount);
    if (kbState[SDL_SCANCODE_W] == 1) {
        app->carVel = v3d_Add(app->carVel, accelForward);
    }

    if (kbState[SDL_SCANCODE_S] == 1) {
        app->carVel = v3d_Sub(app->carVel, accelForward);
    }

    app->carPos = v3d_Add(app->carPos, app->carVel);

    // Friction
    {
        const UnitSpherCoord carDirRight = {
            .azim = app->carDir.azim - (M_PI * 0.5),
            .incl = app->carDir.incl,
        };
        const v3d carToFront = UnitSpherCoord_ToCartesian(app->carDir);
        const v3d carToRight = UnitSpherCoord_ToCartesian(carDirRight);
        const double dotForward = v3d_Dot(carToFront, app->carVel);
        const double dotRight   = v3d_Dot(carToRight, app->carVel);
        app->carVel = v3d_Add(
            v3d_Mul(carToFront, 0.98 * dotForward),
            v3d_Mul(carToRight, 0.94 * dotRight)
        );
    }


    // fprintf(stdout, "Look angles: (%lf, %lf)\n",
    //     app->lookDir.azim, app->lookDir.incl);
    // printf("app->cameraPos: (%lf, %lf, %lf)\n",
    //     app->cameraPos.x, app->cameraPos.y, app->cameraPos.z);

    v3d lookV = UnitSpherCoord_ToCartesian(app->lookDir);
    app->cameraPos = v3d_Sub(app->carPos, v3d_SetMag(lookV, 240.0));
    LookAt(app, app->carPos);

    if (app->cameraPos.z < -5.0) {
        app->cameraPos.z = -5.0;
        LookAt(app, app->carPos);
    }

    // Render

    SDL_RenderClear(app->renderer);

    // if (0) {
    // 	Mix_PlayChannel(-1, app->chunk, 0);
    // }

    for (size_t i = 0; i < app->bufLen; i += 1) {
        app->buf[i] = app->bgBuf[i];
    }

    ResetDepthBuf(app);

    {
        const v3d keep = app->cameraPos;
        const UnitSpherCoord keepDir = app->lookDir;

        app->currentTex = app->currentCarTex;

        app->cameraPos = v3d_Sub(app->cameraPos, app->carPos);

        const double rad = app->carDir.azim;
        app->lookDir.azim -= rad;
        LookUsingUnitSpherCoord(app, app->lookDir);
        app->cameraPos = v3d_RotateZ(
            app->cameraPos, -rad);

        for (size_t i = 0; i < app->carVecTriInfo.length; i += 1) {
            DrawTriInfo(app, &app->carVecTriInfo.buf[i]);
        }

        LookUsingUnitSpherCoord(app, keepDir);
        app->cameraPos = keep;
    }

    const bool canSplatter = v3d_Mag(app->carVel) > 4.0;

    for (int i = 0; i < APP_NUM_BUGS; i += 1)
    {
        if (app->bugs[i].alive && canSplatter) {
            const double d = v3d_Distance(app->carPos, app->bugs[i].pos);
            if (d < 58.0) {
                int s = rand() % APP_NUM_SPLATS;
                if (s == app->prevSplat) {
                    s = (s + 1) % APP_NUM_SPLATS;
                }
                Mix_PlayChannel(-1, app->splats[s], 0);
                app->prevSplat = s;

                if (timeMs < app->scoreEndTimeMs) {
                    app->score += 1;
                }
                app->bugs[i].alive = false;
                app->bugs[i].deadAzim = app->lookDir.azim;
                // To help avoid z-fighting.
                app->bugs[i].pos.z -= (double)rand() / RAND_MAX * 2.0;

                app->carVel = v3d_Mul(app->carVel, 0.99);
            }
        }


        const v3d keep = app->cameraPos;
        const UnitSpherCoord keepDir = app->lookDir;

        if (app->bugs[i].alive) {
            app->currentTex = &app->bugTex;
        }
        else {
            app->currentTex = &app->deadBugTex;
        }

        app->cameraPos = v3d_Sub(app->cameraPos, app->bugs[i].pos);

        // Always face camera.
        double rad;
        if (app->bugs[i].alive) {
            rad = app->lookDir.azim;
        }
        else {
            rad = app->bugs[i].deadAzim;
        }

        app->lookDir.azim -= rad;
        LookUsingUnitSpherCoord(app, app->lookDir);
        app->cameraPos = v3d_RotateZ(
            app->cameraPos, -rad);

        if (app->bugs[i].alive) {
            for (size_t i = 0; i < app->bugVecTriInfo.length; i += 1) {
                DrawTriInfo(app, &app->bugVecTriInfo.buf[i]);
            }
        }
        else {
            for (size_t i = 0; i < app->deadBugVecTriInfo.length; i += 1) {
                DrawTriInfo(app, &app->deadBugVecTriInfo.buf[i]);
            }
        }

        LookUsingUnitSpherCoord(app, keepDir);
        app->cameraPos = keep;
    }

    if (app->transMode) {
        size_t i = 0;
        const double invProg = 1.0 - prog;
        for (size_t p = 0; p < app->numPixels; p += 1) {
            app->buf[i] = (uint8_t)(((double)app->buf[i]) * invProg); i += 1;
            app->buf[i] = (uint8_t)(((double)app->buf[i]) * invProg); i += 1;
            app->buf[i] = (uint8_t)(((double)app->buf[i]) * invProg); i += 1;
            app->buf[i] = 255;      i += 1;
        }
    }

    // Copy buf to background.
    SDL_UpdateTexture(app->texture, NULL, app->buf, app->renderSizePx.x * 4);
    SDL_RenderCopy(app->renderer, app->texture, NULL, NULL);

    if (app->showInfoText) {
        CopyText(app->renderer, &app->infoTRes, 0, 0);
    }

    if (!app->transMode) {
        UpdateScoreTRes(app, timeMs);
        CopyText(app->renderer, &app->scoreTRes,
            app->renderSizePx.x - app->scoreTRes.surface->w - 10,
            10.0
        );
    }

    // // Draw grid of dots.
    // Sdlu_SetRenderDrawColor(app->renderer, 255, 255, 255, 255);
    // for (double y = 0.0; y <= 50.0; y += 2.0) {
    //     for (double x = 0.0; x <= 50.0; x += 2.0) {
    //         const v3d px = WorldToScreen(app, (v3d) { x, y, 0.0 });
    //         if (px.z > 0.0) {
    //             Sdlu_RenderDrawPoint(app->renderer, (int)px.x, (int)px.y);
    //         }
    //     }
    // }

    SDL_RenderPresent(app->renderer);

    // if (app->recording) {
    //     #define APP_FRAME_PATH_LEN 1024
    //     char path[APP_FRAME_PATH_LEN];
    //     snprintf(path, APP_FRAME_PATH_LEN, "screenshots/frame_%ld_%d.bmp",
    //         app->recordingId, app->recordingFrameNum);
    //
    //     Sdlu_RendererSaveBmp(app->renderer, path);
    //
    //     app->recordingFrameNum += 1;
    // }

    end_of_while_loop:
    app->oldTimeMs = timeMs;

    {
        if (prog == 1.0) {
            StartMain(app);
        }
    }

    // Return true to keep the loop running.
    return EM_TRUE;
}

EM_BOOL App_Iterate(double timeMs, void* userData) {
    App *const app = userData;

    switch (app->mode) {
        case APP_MODE_MAIN:
        {
            return App_IterateMain(timeMs, userData);
            break;
        }
        case APP_MODE_GAME:
        {
            return App_IterateGame(timeMs, userData);
            break;
        }
        default:
        {
            fprintf(stderr, "%s: Unknown mode: %d\n", __func__, app->mode);
            exit(1);
        }
    }
}

void App_Deinit(App *const app) {
    // printf("%s\n", __func__);

    SDL_DestroyWindow(app->window);
    SDL_DestroyRenderer(app->renderer);
    SDL_DestroyTexture(app->texture);
    SDL_Quit();

    free(app->buf);

    Mix_CloseAudio();

    TTF_Quit();
    Mix_Quit();
}
