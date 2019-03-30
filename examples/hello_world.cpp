// To build this example:
//
// VC++: Just create an empty project and add this file. No need to link to anything.
// GCC/Clang (Windows) g++ hello_world.cpp -lgdi32
// GCC/Clang (Linux)   g++ hello_world.cpp -lX11 -lXext -lpthread -lm
//
// You shouldn't need to install anything other than the development packages for X11, pthreads, etc., but they
// should all be ubiquitous and quite easy to install.
#define MO_USE_EXTERNAL_MINIAUDIO
#define MINTARO_IMPLEMENTATION
#include "../mintaro.h"

#define SCREEN_MAIN_MENU    0
#define SCREEN_IN_GAME      1
#define SCREEN_OPTIONS      2
int g_CurrentScreen = SCREEN_MAIN_MENU;

int g_FocusedMenuItem = 0;

float g_PlayerPosX = 0;
float g_PlayerPosY = 0;

void example1_on_step(mo_context* pContext, double dt)
{
    int black = mo_find_closest_color(pContext, mo_make_rgb(0, 0, 0));
    int white = mo_find_closest_color(pContext, mo_make_rgb(255, 255, 255));
    int blue  = mo_find_closest_color(pContext, mo_make_rgb(128, 192, 255));

    mo_clear(pContext, black);

    switch (g_CurrentScreen)
    {
        case SCREEN_MAIN_MENU:
        {
            // Input.
            if (mo_was_button_pressed(pContext, MO_BUTTON_DOWN)) {
                g_FocusedMenuItem = (g_FocusedMenuItem + 1) % 3;
            }
            if (mo_was_button_pressed(pContext, MO_BUTTON_UP)) {
                if (g_FocusedMenuItem == 0) {
                    g_FocusedMenuItem = 2;
                } else {
                    g_FocusedMenuItem -= 1;
                }
            }

            if (mo_was_button_pressed(pContext, MO_BUTTON_A) || mo_was_button_pressed(pContext, MO_BUTTON_START)) {
                if (g_FocusedMenuItem == 0) {
                    g_PlayerPosX = (float)(pContext->profile.resolutionX/2 - 16);
                    g_PlayerPosY = (float)(pContext->profile.resolutionY/2 - 16);
                    g_CurrentScreen = SCREEN_IN_GAME;
                } else if (g_FocusedMenuItem == 1) {
                    g_CurrentScreen = SCREEN_OPTIONS;
                } else if (g_FocusedMenuItem == 2) {
                    mo_close(pContext);
                }
            }

            // Graphics.
            int caretPosX = 4;
            int caretPosY = 8 + (12*g_FocusedMenuItem);
            mo_draw_text(pContext, caretPosX, caretPosY, white, ">");
            mo_draw_text(pContext, 15, 8+12*0, white, "Start Game");
            mo_draw_text(pContext, 15, 8+12*1, white, "Options");
            mo_draw_text(pContext, 15, 8+12*2, white, "Quit");
        } break;

        case SCREEN_IN_GAME:
        {
            // Input.
            if (mo_is_button_down(pContext, MO_BUTTON_LEFT)) {
                g_PlayerPosX -= (float)(100 * dt);
            }
            if (mo_is_button_down(pContext, MO_BUTTON_RIGHT)) {
                g_PlayerPosX += (float)(100 * dt);
            }
            if (mo_is_button_down(pContext, MO_BUTTON_UP)) {
                g_PlayerPosY -= (float)(100 * dt);
            }
            if (mo_is_button_down(pContext, MO_BUTTON_DOWN)) {
                g_PlayerPosY += (float)(100 * dt);
            }

            if (mo_was_button_pressed(pContext, MO_BUTTON_START)) {
                g_CurrentScreen = SCREEN_MAIN_MENU;
            }

            // Graphics.
            mo_draw_quad(pContext, (int)g_PlayerPosX, (int)g_PlayerPosY, 32, 32, blue);
            mo_draw_textf(pContext, 4, 4, white, "FPS: %u", (unsigned int)(1/dt));
        } break;

        case SCREEN_OPTIONS:
        {
            // Input.
            if (mo_was_button_pressed(pContext, MO_BUTTON_B)) {
                g_CurrentScreen = SCREEN_MAIN_MENU;
            }

            // Graphics.
            mo_draw_text(pContext, 8, 8, white, "OPTIONS");
            mo_draw_text(pContext, 8, 30, white, "Press 'X' to go");
            mo_draw_text(pContext, 8, 39, white, "back");
        } break;

        default:
        {
            // Unknown state.
        } break;
    }
}

int main(int argc, char** argv)
{
    (void)argc;
    (void)argv;

    mo_context* pContext;
    if (mo_init(NULL, 160*2, 144*2, "Hello, World!", example1_on_step, NULL, &pContext) != MO_SUCCESS) {
        return -1;
    }

    int result = mo_run(pContext);

    mo_uninit(pContext);
    return result;
}
