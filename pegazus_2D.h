#include "device_launch_parameters.h"
#include "cuda_runtime.h"
#include "pegazus_config.h"
#pragma once

PEGA_SHAPE_2D_POINT* points2D;
int points2D_counter;
int points2D_MAX;

PEGA_SHAPE_2D_LINE* lines2D;
int lines2D_counter;
int lines2D_MAX;

PEGA_SHAPE_2D_TRIANGLE* triangles2D;
int triangles2D_counter;
int triangles2D_MAX;

PEGA_SHAPE_2D_LINE* CUDA_lines2D[CUDA_MAX_DEVICES];
PEGA_SHAPE_2D_POINT* CUDA_points2D[CUDA_MAX_DEVICES];
PEGA_SHAPE_2D_TRIANGLE* CUDA_triangles2D[CUDA_MAX_DEVICES];

__device__ void CUDA_SetPixel2D(int x1, int y1, int szin, unsigned int* puffer, unsigned int* maskpuffer);
__device__ void CUDA_DrawLine2D(int x1, int y1, int x2, int y2, int szin, unsigned int* puffer, unsigned int* maskpuffer);
__device__ void CUDA_FillTriangle(int x1, int y1, int x2, int y2, int x3, int y3, int szin, unsigned int* puffer, unsigned int* maskpuffer);

__device__ void CUDA_SetPixel2D(int x1, int y1, int szin, unsigned int* puffer, unsigned int* maskpuffer)
{
 int i;
 puffer[(y1 * KEPERNYO_WIDTH) + x1] = szin;
 maskpuffer[(y1 * KEPERNYO_WIDTH) + x1] = 1;
}

__device__ void CUDA_DrawLine2D(int x1, int y1, int x2, int y2, int szin, unsigned int* puffer, unsigned int* maskpuffer)
{
 bool flip = false;
 int csere, aktoffset;

 if (abs(x2 - x1) < 2 && abs(y2 - y1) < 2)
 {
  aktoffset = (y2 * KEPERNYO_WIDTH);
  puffer[aktoffset + x2] = szin;
  maskpuffer[aktoffset + x2] = 1;
  return;
 }
 if (abs(x1 - x2) < abs(y1 - y2))
 {
  csere = x1;
  x1 = y1;
  y1 = csere;

  csere = x2;
  x2 = y2;
  y2 = csere;
  flip = true;
 }
 if (x1 > x2)
 {
  csere = x1;
  x1 = x2;
  x2 = csere;

  csere = y1;
  y1 = y2;
  y2 = csere;
 }
 int dx = x2 - x1;
 int dy = y2 - y1;

 int jelzo1 = abs(dy) * 2;
 int jelzo2 = 0;
 int y = y1, x;

 if (flip)
 {
  for (x = x1; x <= x2; ++x)
  {
   aktoffset = (x * KEPERNYO_WIDTH);
   puffer[aktoffset + y] = szin;
   maskpuffer[aktoffset + y] = 1;
   jelzo2 += jelzo1;
   if (jelzo2 > dx)
   {
    y += (y2 > y1 ? 1 : -1);
    jelzo2 -= dx * 2;
   }
  }
 }
 else
 {
  for (x = x1; x <= x2; ++x)
  {
   aktoffset = (y * KEPERNYO_WIDTH);
   puffer[aktoffset + x] = szin;
   maskpuffer[aktoffset + y] = 1;
   jelzo2 += jelzo1;
   if (jelzo2 > dx)
   {
    y += (y2 > y1 ? 1 : -1);
    jelzo2 -= dx * 2;
   }
  }
 }
}

__device__ void CUDA_FillTriangle(int x1, int y1, int x2, int y2, int x3, int y3, int szin, unsigned int* puffer, unsigned int* maskpuffer)
{
 int Ax, Ay, Bx, By, i, j;
 int cserex, cserey, aktoffset, maxoffset = KEPERNYO_HEIGHT * KEPERNYO_WIDTH;
 if (y1 == y2 && y1 == y3) return;

 if (y1 > y2)
 {
  cserex = x1;
  cserey = y1;
  x1 = x2;
  y1 = y2;
  x2 = cserex;
  y2 = cserey;
 }
 if (y1 > y3)
 {
  cserex = x1;
  cserey = y1;
  x1 = x3;
  y1 = y3;
  x3 = cserex;
  y3 = cserey;
 }
 if (y2 > y3)
 {
  cserex = x3;
  cserey = y3;
  x3 = x2;
  y3 = y2;
  x2 = cserex;
  y2 = cserey;
 }
 int magassag = y3 - y1;
 for (i = 0; i < magassag; ++i)
 {
  bool alsoresz = i > y2 - y1 || y2 == y1;
  int reszmagassag = alsoresz ? y3 - y2 : y2 - y1;
  float alpha = (float)i / magassag;
  float beta = (float)(i - (alsoresz ? y2 - y1 : 0)) / reszmagassag;
  Ax = x1 + (x3 - x1) * alpha;
  Ay = y1 + (y3 - y1) * alpha;
  Bx = alsoresz ? x2 + (x3 - x2) * beta : x1 + (x2 - x1) * beta;
  By = alsoresz ? y2 + (y3 - y2) * beta : y1 + (y2 - y1) * beta;
  if (Ax > Bx)
  {
   cserex = Ax;
   cserey = Ay;
   Ax = Bx;
   Ay = By;
   Bx = cserex;
   By = cserey;
  }

  aktoffset = (y1 + i) * KEPERNYO_WIDTH;
  for (j = Ax; j < Bx; ++j)
  {
   if (aktoffset + j > maxoffset) continue;
   puffer[aktoffset + j] = szin;
   maskpuffer[aktoffset + j] = 1;
  }
 }
}
