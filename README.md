# Bug Rave

We visit the bug rave.

Click to enter the rave.  
In the rave, click again to reenter zen.  
Drive with WASD.  
Move camera with mouse.  
Hold Shift for Turbo.  
Eliminate the bugs.  
Score will NOT increase after time runs out.

Feel free to open an issue to ask a question.

Dependencies:
- C11 standard library (C11 for `timespec_get`)
- [SDL2](https://www.libsdl.org/index.php) (tested with 2.0.16)
- [SDL_mixer](https://www.libsdl.org/projects/SDL_mixer/) (tested with 2.0.4)
- [SDL_ttf](https://www.libsdl.org/projects/SDL_ttf/) (tested with 2.0.15)
- [stb_image](https://github.com/nothings/stb)
- [stb_image_write](https://github.com/nothings/stb)
- [tinyobjloader in C](https://github.com/syoyo/tinyobjloader-c)

[emscripten](https://emscripten.org/) is used for the web build.

## Build and run

Install the dependencies through your package manager.  
e.g. `pamac install sdl2 sdl2_mixer sdl2_ttf emscripten`

Use the makefile to build and/or run.  
`make build`  
`make run`

The emscripten build is put into `www` with `make emsbuild`.
You will need to serve the build
(you cannot simply open the built `index.html`);

## License

Materials original to this repo are available under the BSD 2-Clause License.
See `LICENSE.txt` for details.

## Contributing

Not currently accepting contributions.
Feel free to open an issue.

## Attribution

Music: [Rave Try (Blueeskies)](https://opengameart.org/content/rave-try-blueeskies)
by [Android128](https://soundcloud.com/android128%C2%A0)
under [CC-BY 3.0](https://creativecommons.org/licenses/by/3.0/)  
[Squish sound effects](https://opengameart.org/content/2-wooden-squish-splatter-sequences)
by Independent.nu
under [CC0](https://creativecommons.org/publicdomain/zero/1.0/)  
[Bug image](https://unsplash.com/photos/NABcJwd6p90)
by [Heiko Haller](https://twitter.com/heikohaller)
under Unsplash License  
[Car side image](https://unsplash.com/photos/N7RiDzfF2iw)
by [Dan Gold](https://www.danielcgold.com/)
under Unsplash License  
[Car front image](https://unsplash.com/photos/J3nHfF6TIwQ)
by [Hrayr Movsisyan](https://unsplash.com/@lincerta)
under Unsplash License  
[Car rear image](https://unsplash.com/photos/ULoBo1cCyEs)
by [Mihail Macri](https://unsplash.com/@macrimihail)
under Unsplash License

[Unsplash License](https://unsplash.com/license):
> Unsplash grants you an irrevocable, nonexclusive, worldwide copyright license to download, copy, modify, distribute, perform, and use photos from Unsplash for free, including for commercial purposes, without permission from or attributing the photographer or Unsplash. This license does not include the right to compile photos from Unsplash to replicate a similar or competing service.
