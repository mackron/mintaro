#define STB_IMAGE_IMPLEMENTATION
#include "../extras/stb_image.h"

#include <string.h>

typedef struct
{
    union
    {
        struct
        {
            unsigned char b;
            unsigned char g;
            unsigned char r;
            unsigned char a;
        };

        unsigned int rgba;
    };
} color_rgba;

int main()
{
    // To generate the palette we just load an image and add each unique color. Color index 255 is overwritten with a transparent color.

	int srcSizeX;
	int srcSizeY;
	stbi_uc* pSrcData = stbi_load("../data/palette.png", &srcSizeX, &srcSizeY, NULL, 4);
	if (pSrcData == NULL) {
		printf("Failed to load font file.");
		return -1;
	}

    int colorCount = 0;
    color_rgba palette[256];
    for (int i = 0; i < 256; ++i) {
        palette[i].r = 0;
        palette[i].g = 0;
        palette[i].b = 0;
        palette[i].a = 255;
    }
    

    for (int i = 0; i < srcSizeX*srcSizeY; ++i) {
        stbi_uc* pixel = pSrcData + (i*4);
        color_rgba color;
        color.r = pixel[0];
        color.g = pixel[1];
        color.b = pixel[2];
        color.a = 255;

        // If the pixel already exists in the palette just skip it.
        int exists = 0;
        for (int j = 0; j < colorCount; ++j) {
            if (palette[j].rgba == color.rgba) {
                exists = 1;
                break;
            }
        }

        if (!exists) {
            palette[colorCount++] = color;
        }
    }

    // Fill any remaining slots with true grays. This assumes true black and true white are already in the palette.
    int grayCount = 256 - colorCount;
    if (grayCount > 0) {
        unsigned char shade = 256 / grayCount;
        color_rgba gray;
        gray.a = 255;
        for (int i = 0; i < grayCount; ++i) {
            gray.r = gray.g = gray.b = (unsigned char)((i+1)*shade);
            palette[colorCount++] = gray;
        }
    }
    

    

    // Transparent color.
    palette[255].r = 0;
    palette[255].g = 0;
    palette[255].b = 0;
    palette[255].a = 0;


    int newLineCounter = 0;
    for (unsigned int i = 0; i < 256; ++i) {
        printf("0x%08X, ", palette[i].rgba);
        //printf("{0x%02X, 0x%02X, 0x%02X, 0x%02X}, ", palette[i].b, palette[i].g, palette[i].r, palette[i].a);
        if (++newLineCounter == 16) {
            printf("\n");
            newLineCounter = 0;
        }
    }

    return 0;
}