#include "App.h"

#include "Clock.h"

///////////////////////////////////////////////////////////////////////////////

App app;

///////////////////////////////////////////////////////////////////////////////

int main(void) {
    puts("Bug Rave by Costava");

    App_Init(&app);

#ifdef __EMSCRIPTEN__
    // Receives a function to call and some user data to provide it.
    emscripten_request_animation_frame_loop(App_Iterate, &app);
#else
    while (!app.quit) {
        const double newTimeMs = Clock_GetTimeMs();
        App_Iterate(newTimeMs, &app);
    }

    App_Deinit(&app);

    return 0;
#endif
}
