// This is just a basic tool for converting the source font image to the native internal format. This
// is probably temporary until it's converted to a bitmap.

#define STB_IMAGE_IMPLEMENTATION
#include "../stb_image.h"

int main()
{
	int srcSizeX;
	int srcSizeY;
	stbi_uc* pSrcData = stbi_load("../data/atari_8bit_font_revised.png", &srcSizeX, &srcSizeY, NULL, 1);
	if (pSrcData == NULL) {
		printf("Failed to load font file.");
		return -1;
	}
	
	int newLineCounter = 0;
	
	#if 1
	int glyphCount = srcSizeX / srcSizeY;
	for (int i = 0; i < glyphCount; ++i) {
		int offset = i*9;
		for (int y = 0; y < 9; ++y) {
			for (int x = 0; x < 9; ++x) {
				printf("0x%02x, ", (pSrcData + offset)[y*srcSizeX + x]);
				if (++newLineCounter == 36) {
					printf("\n");
					newLineCounter = 0;
				}
			}
		}
	}
	#endif
	
	#if 0
	stbi_uc* pRunningPixel = pSrcData;
	for (int y = 0; y < srcSizeY; ++y) {
		for (int x = 0; x < srcSizeX; ++x) {
			printf("0x%02x, ", pRunningPixel[0]);
			if (++newLineCounter == 36) {
				printf("\n");
				newLineCounter = 0;
			}
			
			pRunningPixel += 4;
		}
	}
	#endif
	
	return 0;
} 