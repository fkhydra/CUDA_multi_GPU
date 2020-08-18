#pragma once
#include <stdio.h>
#include <d2d1.h>

void PEGA_load_BMP(const char* fajlnev, unsigned int* picbuffer);

void PEGA_load_BMP(const char *fajlnev, unsigned int* picbuffer)
{
 FILE* bmpfajl;
 int i, j, s = 0;
 unsigned char R, G, B;
 bmpfajl = fopen(fajlnev, "rb");
 if (bmpfajl == NULL) return;
 fseek(bmpfajl, 54, SEEK_SET);
 for (i = 0; i < 1080; ++i)
  for (j = 0; j < 1920; ++j)
  {
   fread(&B, 1, 1, bmpfajl);
   fread(&G, 1, 1, bmpfajl);
   fread(&R, 1, 1, bmpfajl);
   picbuffer[s++] = RGB(B, G, R);
  }
 fclose(bmpfajl);
}
