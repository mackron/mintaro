// This is just a basic tool for print the a-law and mu-law tables.
#include <stdio.h>

int main()
{
    // a-law.
    printf("A-LAW\n");
    {
        int newLineCounter = 0;
        for (unsigned int i = 0; i < 256; ++i)
        {
            const unsigned char a = (unsigned char)i ^ 0x55;

            int t = (a & 0x0F) << 4;

            int s = ((unsigned int)a & 0x70) >> 4;
            switch (s)
            {
                case 0:
                {
                    t += 8;
                } break;

                default:
                {
                    t += 0x108;
                    t <<= (s - 1);
                } break;
            }

            if ((a & 0x80) == 0) {
                t = -t;
            }

            printf("0x%04X, ", (unsigned short)(t));
            if (++newLineCounter == 16) {
                printf("\n");
                newLineCounter = 0;
            }
        }
    }
    printf("\n");

    printf("U-LAW\n");
    {
        int newLineCounter = 0;
        for (unsigned int i = 0; i < 256; ++i)
        {
            const unsigned char u = ~i;

            int t = (((u & 0x0F) << 3) + 0x84) << (((unsigned int)u & 0x70) >> 4);
            if (u & 0x80) {
                t = 0x84 - t;
            } else {
                t = t - 0x84;
            }

            printf("0x%04X, ", (unsigned short)(t));
            if (++newLineCounter == 16) {
                printf("\n");
                newLineCounter = 0;
            }
        }
    }
    printf("\n");
	
	return 0;
} 