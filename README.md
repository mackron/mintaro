![Mintaro](http://dred.io/img/mintaro_wide.png)

Mintaro is a tiny framework for making simple, retro style games. It's not intended to be
a full-featured game engine, but is instead focused on simplicity and just making it fun
to make simple games.

C/C++, single file, public domain.


Features
========
- A single file with optional dependencies to extend functionality.
- No external dependencies except for the standard library and necessary platform libraries
  like XLib and Win32.
- Software rendering, with up to 256 colors and a customizable palette.
- Uncapped framerate.
- Custom resolutions of any dimensions.
- 8 buttons of input
  - Up, down, left, right
  - A, B
  - Start, Select
- Sound groups with independant volume controls.
- A simple API.
- Supports Windows and Linux.


Features Coming Soon
====================
- Fullscreen mode
- Line rasterization
- Triangle rasterization (solid and textured)
- Rotated sprites
- More optimizations, especially for graphics
- More platforms
- More flexibility for input:
  - Support for binding different keys to the same button.
  - Support for general keyboard controls (in addition to buttons)
  - Support for mouse controls
  - Support for 360 controllers


Usage
=====
Mintaro is a single-file library. To use it, just #include "mintaro.h" like you would any other
header file and then in one source file do the following:

    #define MINTARO_IMPLEMENTATION
    #include "mintaro.h"
    
Make sure you don't define the implementation in more than one translation unit.

Mintaro includes a built-in loader for TGA images, but you can enable loading of additional
formats via stb_image by simply including it before the implementation of Mintaro, like this:

    #define STB_IMAGE_IMPLEMENTATION
    #include "stb_image.h"
    
    #define MINTARO_IMPLEMENTATION
    #include "mintaro.h"
    
A copy of stb_image.h is included in the "extras" directory.

Mintaro includes a built-in loader for WAV sounds, but you can enable loading of Vorbis and FLAC
sounds by #including stb_vorbis.c and/or dr_flac.h before the implementation of Mintaro, in the
same was as mentioned above for stb_image.h.


Examples
========
Mintaro is focused on simplicity. Here's a quick example.

```c
void on_step(mo_context* pContext, double dt)
{
    // Input.
    if (mo_was_button_pressed(MO_BUTTON_A)) {
        if (CanShoot()) {
            mo_play_sound_source(pContext, pGunshotSoundSource);
        }
    }
    
    // Simulation.
    UpdateCharacter(dt);

    // Drawing.
    mo_clear(pContext, clearColorIndex);
    mo_draw_image(pContext, characterPosX, characterPosY, pCharacterSpriteSheet, 0, 0, 64, 64);
    mo_draw_textf(pContext, 0, 0, textColorIndex, "Health: %d", characterHealth);
}

int main()
{
    mo_context* pContext;
    if (mo_init(NULL, "My Game's Name", on_step, pUserData, &pContext) {
        return -1;
    }
    
    // Load some resources.
    mo_sound_source* pMusicSource;
    mo_sound_source_load(pContext, "data/my_music.wav", &pMusic);
    
    mo_image* pCharacterSpriteSheet;
    mo_image_load(pContext, "data/character.tga", &pCharacterSpriteSheet);
    
    
    // Play some music in a loop.
    mo_sound* pMusic;
    mo_sound_create(pContext, pMusicSource, NULL, &pMusic);
    mo_sound_play(pMusic, MO_TRUE);
    
    int result = mo_run(pContext);
    
    mo_uninit(pContext);
    return result;
}
```

You can also find a "Hello, World!" example in the "examples" folder.