#pragma once
#include "pegazus_config.h"

Vec3f vLight;
bool enable_lighting;

float* CUDA_Zbuffer[CUDA_MAX_DEVICES], * CUDA_DEV0_COPY_Zbuffer;

float Math_PI = 3.14159265358979323846;

float nagyitas_ertek;
int render_inprogress;
float persp_szog;

Vec3f fszog, fszog2;
Vec3f* nyers_csucspontok, * CUDA_nyers_csucspontok[CUDA_MAX_DEVICES], * CUDA_forgatott_pontok[CUDA_MAX_DEVICES];
int nyers_csucspontok_length, MAX_3d_vertex_count;

//*******statisztika keszitesehez********
long int szamlalo_vert, szamlalo_poly;
float fps_stat;
int kezdet;
int vege;

void PEGA_init_3D(void);
void PEGA_3D_vertex_reset(void);
void PEGA_create_HOST_CUDA_3D_vertex_list(int count);
void PEGA_free3D(void);
void PEGA_rotate_3D(void);
void PEGA_render_3D(void);
void PEGA_merge_down_zbuffers(void);
__global__ void CUDA_Merge_Zbuffers(float* main_zpuffer, float* secondary_zpuffer, unsigned int* main_kepadat, unsigned int* secondary_kepadat);
__global__ void PEGA_3D_rotation(int maxelemszam, Vec3f* nyerstomb, Vec3f* forgtomb, Vec3f szog_cos, Vec3f szog_sin);
void PEGA_add_3D_triangle(int x1, int y1, int z1, int x2, int y2, int z2, int x3, int y3, int z3);
void PEGA_push_3D_triangles_to_GPU(void);
void PEGA_clear_zbuffer(void);
__global__ void PEGA_cleanup_zbuffer(float* zpuffer);
__global__ void PEGA_render_objects(int maxelemszam, Vec3f* forgtomb, unsigned int* puffer, float* zpuffer, Vec3f fenyvektor, bool islightenabled);
__device__ void CUDA_SetPixel_Zbuffer(int x1, int y1, int z1, int szin, unsigned int* puffer, float* zpuffer);
__device__ void CUDA_DrawLine_Zbuffer(int x1, int y1, int z1, int x2, int y2, int z2, int szin, unsigned int* puffer, float* zpuffer);
__device__ void CUDA_FillTriangle_Zbuffer(int x1, int y1, int z1, int x2, int y2, int z2, int x3, int y3, int z3, int szin, unsigned int* puffer, float* zpuffer);
void PEGA_zoom_in(void);
void PEGA_zoom_out(void);
__global__ void CUDA_zoom_in(int maxelemszam, Vec3f* nyerstomb);
__global__ void CUDA_zoom_out(int maxelemszam, Vec3f* nyerstomb);
void PEGA_start_fps_benchmark(void);
void PEGA_get_fps_benchmark(void);

void PEGA_init_3D(void)
{
 enable_lighting = true;
 szamlalo_vert = szamlalo_vert = 0;
 MAX_3d_vertex_count = 0;
 nagyitas_ertek = 1.0;
 render_inprogress = 0;
 persp_szog = Math_PI / 180;
 fszog.x = 0 * Math_PI / 180; fszog2.x = 0;
 fszog.y = 0 * Math_PI / 180; fszog2.y = 0;
 fszog.z = 0 * Math_PI / 180; fszog2.z = 0;
 nyers_csucspontok_length = 0;
 vLight.x = -0.5;
 vLight.y = -0.5;
 vLight.z = -0.9;
}

void PEGA_3D_vertex_reset(void)
{
 nyers_csucspontok_length = 0;
}

void PEGA_create_HOST_CUDA_3D_vertex_list(int count)
{
 int i;
 MAX_3d_vertex_count = count;
 nyers_csucspontok_length = 0;
 nyers_csucspontok = (Vec3f*)malloc(MAX_3d_vertex_count * sizeof(Vec3f));

 cudaSetDevice(0);
 cudaMalloc((void**)&CUDA_DEV0_COPY_Zbuffer, KEPERNYO_WIDTH * KEPERNYO_HEIGHT * sizeof(float));

 for (i = 0; i < CUDA_DEVICE_COUNT; ++i)
 {
  cudaSetDevice(i);
  cudaMalloc((void**)&CUDA_nyers_csucspontok[i], (MAX_3d_vertex_count / CUDA_DEVICE_COUNT) * sizeof(Vec3f));
  cudaMalloc((void**)&CUDA_forgatott_pontok[i], (MAX_3d_vertex_count / CUDA_DEVICE_COUNT) * sizeof(Vec3f));
  cudaMalloc((void**)&CUDA_Zbuffer[i], KEPERNYO_WIDTH * KEPERNYO_HEIGHT * sizeof(float));
 }
}

void PEGA_free3D(void)
{
 int i;
 free(nyers_csucspontok);
 cudaSetDevice(0);
 cudaFree(CUDA_DEV0_COPY_Zbuffer);
 for (i = 0; i < CUDA_DEVICE_COUNT; ++i)
 {
  cudaSetDevice(i);
  cudaFree(CUDA_nyers_csucspontok[i]);
  cudaFree(CUDA_forgatott_pontok[i]);
  cudaFree(CUDA_Zbuffer[i]);
 }
}

void PEGA_rotate_3D(void)
{
 int i;
 int blockSize = CUDA_CORES;
 int numBlocks;
 Vec3f szog_sin, szog_cos;

 fszog.x = fszog2.x * Math_PI / 180;
 fszog.y = fszog2.y * Math_PI / 180;
 fszog.z = fszog2.z * Math_PI / 180;
 szog_sin.x = sin(fszog.x);
 szog_cos.x = cos(fszog.x);
 szog_sin.y = sin(fszog.y);
 szog_cos.y = cos(fszog.y);
 szog_sin.z = sin(fszog.z);
 szog_cos.z = cos(fszog.z);

 for (i = 0; i < CUDA_DEVICE_COUNT; ++i)
 {
  if (PEGA_DEVICES.devinfo[i].work_items[PEGA3DTRIANGLE] > 0)
  {
   numBlocks = ((PEGA_DEVICES.devinfo[i].work_items[PEGA3DTRIANGLE] * 3) + blockSize - 1) / blockSize;
   cudaSetDevice(i);
   PEGA_3D_rotation << <numBlocks, blockSize >> > (PEGA_DEVICES.devinfo[i].work_items[PEGA3DTRIANGLE] * 3, CUDA_nyers_csucspontok[i], CUDA_forgatott_pontok[i], szog_cos, szog_sin);
  }
 }
 cudaDeviceSynchronize();
}

void PEGA_render_3D(void)
{
 int i;
 int blockSize = CUDA_CORES;
 int numBlocks;
 PEGA_clear_zbuffer();

 for (i = 0; i < CUDA_DEVICE_COUNT; ++i)
 {
  if (PEGA_DEVICES.devinfo[i].work_items[PEGA3DTRIANGLE] > 0)
  {
   numBlocks = ((PEGA_DEVICES.devinfo[i].work_items[PEGA3DTRIANGLE] * 3) + blockSize - 1) / blockSize;
   cudaSetDevice(i);
   //más típusú alakzatoknál a 3-val történő szorzásra figyelni (modositani kell)
   PEGA_render_objects << <numBlocks, blockSize >> > ((PEGA_DEVICES.devinfo[i].work_items[PEGA3DTRIANGLE] * 3), CUDA_forgatott_pontok[i], CUDA_imagebuffer[i], CUDA_Zbuffer[i], vLight, enable_lighting);
  }
 }
 cudaDeviceSynchronize();
}

void PEGA_merge_down_zbuffers(void)
{
 int i;
 int blockSize = CUDA_CORES;
 int numBlocks;

 cudaSetDevice(0);
 if (CUDA_DEVICE_COUNT > 1)
 {
  for (i = 1; i < CUDA_DEVICE_COUNT; ++i)
  {
   if (PEGA_DEVICES.devinfo[i].work_items[PEGA3DTRIANGLE] > 0)
   {
    cudaMemcpyPeerAsync(CUDA_DEV0_COPY_Zbuffer, 0, CUDA_Zbuffer[i], i, KEPERNYO_WIDTH * KEPERNYO_HEIGHT * sizeof(float));
    cudaMemcpyPeerAsync(CUDA_DEV0_COPY_imagebuffer, 0, CUDA_imagebuffer[i], i, KEPERNYO_WIDTH * KEPERNYO_HEIGHT * sizeof(unsigned int));
    numBlocks = ((PEGA_DEVICES.devinfo[i].work_items[PEGA3DTRIANGLE] * 3) + blockSize - 1) / blockSize;    
    CUDA_Merge_Zbuffers << <numBlocks, blockSize >> > (CUDA_Zbuffer[0], CUDA_DEV0_COPY_Zbuffer, CUDA_imagebuffer[0], CUDA_DEV0_COPY_imagebuffer);
    //cudaDeviceSynchronize();
   }
  }
  cudaDeviceSynchronize();
 }
}

__global__ void CUDA_Merge_Zbuffers(float* main_zpuffer, float* secondary_zpuffer, unsigned int* main_kepadat, unsigned int* secondary_kepadat)
{
 int i;
 int index = (blockIdx.x * blockDim.x) + threadIdx.x;
 int stride = blockDim.x * gridDim.x;

 for (i = index; i < KEPERNYO_HEIGHT * KEPERNYO_WIDTH; i += stride)
 {
  if (main_zpuffer[i] > secondary_zpuffer[i])
  {
   main_kepadat[i] = secondary_kepadat[i];
  }
 }
}

__global__ void PEGA_render_objects(int maxelemszam, Vec3f* forgtomb, unsigned int* puffer, float* zpuffer, Vec3f fenyvektor, bool islightenabled)
{
 int i, tesztszin;
 int index = (blockIdx.x * blockDim.x) + (threadIdx.x * 3);
 int stride = blockDim.x * gridDim.x;
 Vec3f Vector1, Vector2, vNormal;//láthatóságvizsgálathoz
 Vec3f vNormalized;//dinamikus megvilágításhoz
 float Light_intensity, Vector_length;

 for (i = index; i < maxelemszam - 3; i += stride)
 {
  if ((forgtomb[i].z < -9000000) || (forgtomb[i + 1].z < -9000000) || (forgtomb[i + 2].z < -9000000)) continue;

  // láthatóságvizsgálat
  Vector1.x = forgtomb[i + 1].x - forgtomb[i].x;
  Vector1.y = forgtomb[i + 1].y - forgtomb[i].y;
  Vector1.z = forgtomb[i + 1].z - forgtomb[i].z;
  Vector2.x = forgtomb[i + 2].x - forgtomb[i].x;
  Vector2.y = forgtomb[i + 2].y - forgtomb[i].y;
  Vector2.z = forgtomb[i + 2].z - forgtomb[i].z;
  vNormal.x = ((Vector1.y * Vector2.z) - (Vector1.z * Vector2.y));
  vNormal.y = ((Vector1.z * Vector2.x) - (Vector1.x * Vector2.z));
  vNormal.z = ((Vector1.x * Vector2.y) - (Vector1.y * Vector2.x));
  if (vNormal.z > 0) continue;
  if (islightenabled == true)
  {
   Vector_length = sqrtf((vNormal.x * vNormal.x) + (vNormal.y * vNormal.y) + (vNormal.z * vNormal.z));
   vNormalized.x = vNormal.x / Vector_length;
   vNormalized.y = vNormal.y / Vector_length;
   vNormalized.z = vNormal.z / Vector_length;

   Light_intensity = ((vNormalized.x * fenyvektor.x) + (vNormalized.y * fenyvektor.y) + (vNormalized.z * fenyvektor.z));
   if (Light_intensity > 1) Light_intensity = 1;
   else if (Light_intensity < 0) Light_intensity = 0;
   tesztszin = RGB(255 * Light_intensity, 255 * Light_intensity, 255 * Light_intensity);
  }
  else tesztszin = RGB(200, 0, 111);
  /*CUDA_SetPixel_Zbuffer(forgtomb[i].x, forgtomb[i].y, forgtomb[i].z, tesztszin, puffer, zpuffer);
  CUDA_SetPixel_Zbuffer(forgtomb[i + 1].x, forgtomb[i + 1].y, forgtomb[i + 1].z, tesztszin, puffer, zpuffer);
  CUDA_SetPixel_Zbuffer(forgtomb[i + 2].x, forgtomb[i + 2].y, forgtomb[i + 2].z, tesztszin, puffer, zpuffer);*/
  /*CUDA_DrawLine_Zbuffer(forgtomb[i].x, forgtomb[i].y, forgtomb[i].z, forgtomb[i + 1].x, forgtomb[i + 1].y, forgtomb[i + 1].z, tesztszin, puffer, zpuffer);
  CUDA_DrawLine_Zbuffer(forgtomb[i + 2].x, forgtomb[i + 2].y, forgtomb[i + 2].z, forgtomb[i + 1].x, forgtomb[i + 1].y, forgtomb[i + 1].z, tesztszin, puffer, zpuffer);
  CUDA_DrawLine_Zbuffer(forgtomb[i].x, forgtomb[i].y, forgtomb[i].z, forgtomb[i + 2].x, forgtomb[i + 2].y, forgtomb[i + 2].z, tesztszin, puffer, zpuffer);*/
  CUDA_FillTriangle_Zbuffer(forgtomb[i].x, forgtomb[i].y, forgtomb[i].z, forgtomb[i + 1].x, forgtomb[i + 1].y, forgtomb[i + 1].z, forgtomb[i + 2].x, forgtomb[i + 2].y, forgtomb[i + 2].z, tesztszin, puffer, zpuffer);

 }
}

__global__ void PEGA_3D_rotation(int maxelemszam, Vec3f* nyerstomb, Vec3f* forgtomb, Vec3f szog_cos, Vec3f szog_sin)
{
 int i;
 int index = blockIdx.x * blockDim.x + threadIdx.x;
 int stride = blockDim.x * gridDim.x;
 float t0;

 for (i = index; i < maxelemszam; i += stride)
 {
  forgtomb[i].y = (nyerstomb[i].y * szog_cos.x) - (nyerstomb[i].z * szog_sin.x);
  forgtomb[i].z = nyerstomb[i].y * szog_sin.x + nyerstomb[i].z * szog_cos.x;

  forgtomb[i].x = nyerstomb[i].x * szog_cos.y + forgtomb[i].z * szog_sin.y;
  forgtomb[i].z = -nyerstomb[i].x * szog_sin.y + forgtomb[i].z * szog_cos.y;

  t0 = forgtomb[i].x;
  forgtomb[i].x = t0 * szog_cos.z - forgtomb[i].y * szog_sin.z;
  forgtomb[i].y = t0 * szog_sin.z + forgtomb[i].y * szog_cos.z;
 }

 //perspektivikus lekepezes
 int nezopont = -1100;
 float sx = KEPERNYO_WIDTH / 2;
 float sultra = KEPERNYO_HEIGHT / 2, sultra2 = KEPERNYO_HEIGHT / 3;
 int x_minusz_hatar = 0, y_minusz_hatar = 0, x_max_hatar = KEPERNYO_WIDTH - 1, y_max_hatar = KEPERNYO_HEIGHT - 1;
 float tavolsag;

 for (i = index; i < maxelemszam; i += stride)
 {
  tavolsag = 999999;

  if (forgtomb[i].z < tavolsag) tavolsag = forgtomb[i].z;
  if (tavolsag < nezopont) { forgtomb[i].z = -9999999; continue; }//nem jelenitheto meg
  sultra = nezopont / (nezopont - forgtomb[i].z);
  forgtomb[i].x = forgtomb[i].x * sultra + 400 + 400;
  forgtomb[i].y = (forgtomb[i].y * sultra) + sultra2 + 200;
  if (forgtomb[i].x < x_minusz_hatar || forgtomb[i].x > x_max_hatar) { forgtomb[i].z = -9999999; continue; }//nem jelenitheto meg
  if (forgtomb[i].y < y_minusz_hatar || forgtomb[i].y > y_max_hatar) { forgtomb[i].z = -9999999; continue; }//nem jelenitheto meg
 }
}

void PEGA_add_3D_triangle(int x1, int y1, int z1, int x2, int y2, int z2, int x3, int y3, int z3)
{
 if (nyers_csucspontok_length + 3 >= MAX_3d_vertex_count) return;
 nyers_csucspontok[nyers_csucspontok_length].x = x1;
 nyers_csucspontok[nyers_csucspontok_length].y = y1;
 nyers_csucspontok[nyers_csucspontok_length++].z = z1;
 nyers_csucspontok[nyers_csucspontok_length].x = x2;
 nyers_csucspontok[nyers_csucspontok_length].y = y2;
 nyers_csucspontok[nyers_csucspontok_length++].z = z2;
 nyers_csucspontok[nyers_csucspontok_length].x = x3;
 nyers_csucspontok[nyers_csucspontok_length].y = y3;
 nyers_csucspontok[nyers_csucspontok_length++].z = z3;
}

void PEGA_push_3D_triangles_to_GPU(void)
{
 int i;
 if (nyers_csucspontok_length > 0) {
  PEGA_calculate_load_distribution(nyers_csucspontok_length / 3, PEGA3DTRIANGLE);//ez fontos rész, mert ezzel más tipusu alakzatok is elokeszithetoek
  for (i = 0; i < CUDA_DEVICE_COUNT; ++i)
  {
   if (PEGA_DEVICES.devinfo[i].work_items[PEGA3DTRIANGLE] > 0)
   {
    cudaSetDevice(i);
    //más típusú alakzatoknál a 3-val történő szorzásra figyelni (modositani kell)
    cudaMemcpy(CUDA_nyers_csucspontok[i], nyers_csucspontok + (3 * PEGA_DEVICES.devinfo[i].start_item[PEGA3DTRIANGLE]), (3 * PEGA_DEVICES.devinfo[i].work_items[PEGA3DTRIANGLE]) * sizeof(Vec3f), cudaMemcpyHostToDevice);
   }
  }
 }
}

void PEGA_clear_zbuffer(void)
{
 int i;
 for (i = 0; i < CUDA_DEVICE_COUNT; ++i)
 {
  cudaSetDevice(i);
  PEGA_cleanup_zbuffer << < ((KEPERNYO_WIDTH * KEPERNYO_HEIGHT) + CUDA_CORES - 1) / CUDA_CORES, CUDA_CORES >> > (CUDA_Zbuffer[i]);
 }
 cudaDeviceSynchronize();
}

__global__ void PEGA_cleanup_zbuffer(float* zpuffer)
{
 int i;
 int index = (blockIdx.x * blockDim.x) + threadIdx.x;
 int stride = blockDim.x * gridDim.x;

 for (i = index; i < KEPERNYO_HEIGHT * KEPERNYO_WIDTH; i += stride)
 {
  zpuffer[i] = 999999;
 }
}

__device__ void CUDA_SetPixel_Zbuffer(int x1, int y1, int z1, int szin, unsigned int* puffer, float* zpuffer)
{
 int aktoffset = (y1 * KEPERNYO_WIDTH) + x1;
 if (zpuffer[aktoffset] > z1)
 {
  zpuffer[aktoffset] = z1;
  puffer[aktoffset] = szin;
 }
}

__device__ void CUDA_DrawLine_Zbuffer(int x1, int y1, int z1, int x2, int y2, int z2, int szin, unsigned int* puffer, float* zpuffer)
{
 float Pz;
 bool flip = false;
 int csere, aktoffset;

 if (abs(x2 - x1) < 2 && abs(y2 - y1) < 2) {
  puffer[(y2 * KEPERNYO_WIDTH) + x2] = szin; return;
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

 for (x = x1; x <= x2; ++x)
 {
  if (z1 == z2) Pz = z1;
  else
  {
   int s1 = abs(x2 - x1);
   int s2 = abs(z1 - z2);
   Pz = (float)z2 + (float)((((float)x - (float)x1) / (float)s1) * (float)s2);
  }
  if (flip)
  {
   aktoffset = (x * KEPERNYO_WIDTH);
   if (zpuffer[aktoffset + y] > Pz)
   {
    zpuffer[aktoffset + y] = Pz;
    puffer[aktoffset + y] = szin;
   }
  }
  else
  {
   aktoffset = (y * KEPERNYO_WIDTH);
   if (zpuffer[aktoffset + x] > Pz)
   {
    zpuffer[aktoffset + x] = Pz;
    puffer[aktoffset + x] = szin;
   }
  }
  jelzo2 += jelzo1;
  if (jelzo2 > dx)
  {
   y += (y2 > y1 ? 1 : -1);
   jelzo2 -= dx * 2;
  }
 }
}

__device__ void CUDA_FillTriangle_Zbuffer(int x1, int y1, int z1, int x2, int y2, int z2, int x3, int y3, int z3, int szin, unsigned int* puffer, float* zpuffer)
{
 int Ax, Ay, Bx, By, i, j, melysegertek;
 int cserex, cserey, aktoffset;
 Vec3f interpolal, segedvektor;
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
  bool second_half = i > y2 - y1 || y2 == y1;
  int segment_height = second_half ? y3 - y2 : y2 - y1;
  float alpha = (float)i / magassag;
  float beta = (float)(i - (second_half ? y2 - y1 : 0)) / segment_height;
  Ax = x1 + (x3 - x1) * alpha;
  Ay = y1 + (y3 - y1) * alpha;
  Bx = second_half ? x2 + (x3 - x2) * beta : x1 + (x2 - x1) * beta;
  By = second_half ? y2 + (y3 - y2) * beta : y1 + (y2 - y1) * beta;
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
  for (j = Ax; j <= Bx; ++j)
  {
   segedvektor.x = (x2 - x1) * (y1 - (y1 + i)) - (x1 - j) * (y2 - y1);
   segedvektor.y = (x1 - j) * (y3 - y1) - (x3 - x1) * (y1 - (y1 + i));
   segedvektor.z = (x3 - x1) * (y2 - y1) - (x2 - x1) * (y3 - y1);
   if (abs((int)segedvektor.z) < 1) { interpolal.x = -1; interpolal.y = 0; interpolal.z = 0; }
   else
   {
    interpolal.x = 1.f - (segedvektor.x + segedvektor.y) / segedvektor.z;
    interpolal.y = segedvektor.y / segedvektor.z;
    interpolal.z = segedvektor.x / segedvektor.z;
   }
   if (interpolal.x < 0 || interpolal.y < 0 || interpolal.z < 0) continue;
   melysegertek = (z1 * interpolal.x) + (z2 * interpolal.y) + (z3 * interpolal.z);
   if (zpuffer[aktoffset + j] > melysegertek)
   {
    zpuffer[aktoffset + j] = melysegertek;
    puffer[aktoffset + j] = szin;
   }
  }
 }
}

void PEGA_zoom_in(void)
{
 int i;
 int blockSize = CUDA_CORES;
 int numBlocks;
 for (i = 0; i < CUDA_DEVICE_COUNT; ++i)
 {
  if (PEGA_DEVICES.devinfo[i].work_items[PEGA3DTRIANGLE] > 0)
  {
   numBlocks = ((PEGA_DEVICES.devinfo[i].work_items[PEGA3DTRIANGLE] * 3) + blockSize - 1) / blockSize;
   cudaSetDevice(i);
   CUDA_zoom_in << <numBlocks, blockSize >> > (PEGA_DEVICES.devinfo[i].work_items[PEGA3DTRIANGLE] * 3, CUDA_nyers_csucspontok[i]);
  }
 }
 cudaDeviceSynchronize();
}

void PEGA_zoom_out(void)
{
 int i;
 int blockSize = CUDA_CORES;
 int numBlocks;
 for (i = 0; i < CUDA_DEVICE_COUNT; ++i)
 {
  if (PEGA_DEVICES.devinfo[i].work_items[PEGA3DTRIANGLE] > 0)
  {
   numBlocks = ((PEGA_DEVICES.devinfo[i].work_items[PEGA3DTRIANGLE] * 3) + blockSize - 1) / blockSize;
   cudaSetDevice(i);
   CUDA_zoom_out << <numBlocks, blockSize >> > (PEGA_DEVICES.devinfo[i].work_items[PEGA3DTRIANGLE] * 3, CUDA_nyers_csucspontok[i]);
  }
 }
 cudaDeviceSynchronize();
}

__global__ void CUDA_zoom_in(int maxelemszam, Vec3f* nyerstomb)
{
 int i;
 int index = (blockIdx.x * blockDim.x) + threadIdx.x;
 int stride = blockDim.x * gridDim.x;
 for (i = index; i < maxelemszam; i += stride)
 {
  nyerstomb[i].x *= 1.2;
  nyerstomb[i].y *= 1.2;
  nyerstomb[i].z *= 1.2;
 }
}

__global__ void CUDA_zoom_out(int maxelemszam, Vec3f* nyerstomb)
{
 int i;
 int index = (blockIdx.x * blockDim.x) + threadIdx.x;
 int stride = blockDim.x * gridDim.x;
 for (i = index; i < maxelemszam; i += stride)
 {
  nyerstomb[i].x /= 1.2;
  nyerstomb[i].y /= 1.2;
  nyerstomb[i].z /= 1.2;
 }
}

void PEGA_start_fps_benchmark(void)
{
 kezdet = GetTickCount();
}

void PEGA_get_fps_benchmark(void)
{
 vege = GetTickCount();
 if ((vege - kezdet) == 0) ++vege;
 fps_stat = 1000 / (vege - kezdet);
}
