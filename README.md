Mintaro is a tiny framework for making tiny, retro style games.

Features
========
- A single file with optional dependencies to extend functionality.
- No external dependencies except for the standard library and necessary platform libraries such as XLib and Win32.
- Software rendering, with up to 256 colors.
- Uncapped framerate
- Custom resolutions
- 8 bits of input
  - Up, down, left, right
  - A, B
  - Start, Select
- A simple API
- Supports Windows and Linux


Features Coming Soon
====================
- Line rasterization
- Triangle rasterization (solid and textured)
- Rotated sprites
- More optimizations, especially for graphics
- More platforms


Usage
=====
Mintaro is a single-file library. To use it, just #include "mintaro.h" like you would any other header file and then
in one source file do the following:

    #define MINTARO_IMPLEMENTATION
    #include "mintaro.h"
    
Make sure you don't define the implementation in more than one translation unit.

Mintaro includes a built-in loader for TGA images, but you can enable loading of additional formats via stb_image by
simply including it before the implementation of Mintaro, like this:

    #define STB_IMAGE_IMPLEMENTATION
    #include "stb_image.h"
    
    #define MINTARO_IMPLEMENTATION
    #include "mintaro.h"
    
A copy of stb_image.h is included in the "extras" directory.


Examples
========
Mintaro is focused on simplicity. Here's a quick example.

```
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