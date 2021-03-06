#include "cuda_runtime.h"
#include "device_launch_parameters.h"
#include <math.h>
#include <stdio.h>
#include <time.h> 
#include <windows.h>
#include <d2d1.h>
#include <d2d1helper.h>
#pragma comment(lib, "d2d1")
#include <mmsystem.h>
#pragma comment(lib, "winmm.lib")
#include "pegazus_main.h"
#include "PEGA_FORMAT_OBJ_NEW.h"
#include "PEGA_FORMAT_BMP.h"

//***********STANDARD WIN32API ABLAKKEZELES************
HINSTANCE hInstGlob;
int SajatiCmdShow;
HWND Form1; //Ablak kezeloje
LRESULT CALLBACK WndProc0(HWND, UINT, WPARAM, LPARAM);
//******************************************************

//************************
void PEGA_drawing3D_update(void);
//************************************

//*********************************
//A windows program belépési pontja
//*********************************
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR szCmdLine, int iCmdShow)
{
 MSG msg;
 WNDCLASS wndclass0;
 SajatiCmdShow = iCmdShow;
 hInstGlob = hInstance;

 //*********************************
 //Ablak osztálypéldány elokészítése
 //*********************************
 wndclass0.style = CS_HREDRAW | CS_VREDRAW;
 wndclass0.lpfnWndProc = WndProc0;
 wndclass0.cbClsExtra = 0;
 wndclass0.cbWndExtra = 0;
 wndclass0.hInstance = hInstance;
 wndclass0.hIcon = LoadIcon(NULL, IDI_APPLICATION);
 wndclass0.hCursor = LoadCursor(NULL, IDC_ARROW);
 wndclass0.hbrBackground = (HBRUSH)GetStockObject(LTGRAY_BRUSH);
 wndclass0.lpszMenuName = NULL;
 wndclass0.lpszClassName = TEXT("WIN0");

 //*********************************
 //Ablak osztálypéldány regisztrációja
 //*********************************
 if (!RegisterClass(&wndclass0))
 {
  MessageBox(NULL, TEXT("Error:Program initialisation process."), TEXT("Program Start"), MB_ICONERROR);
  return 0;
 }

 //*********************************
 //Ablak létrehozása
 //*********************************
 Form1 = CreateWindow(TEXT("WIN0"),
  TEXT("CUDA - DIRECT2D"),
  (WS_OVERLAPPED | WS_SYSMENU | WS_THICKFRAME | WS_MAXIMIZEBOX | WS_MINIMIZEBOX),
  0,
  0,
  KEPERNYO_WIDTH,
  KEPERNYO_HEIGHT,
  NULL,
  NULL,
  hInstance,
  NULL);

 //*********************************
 //Ablak megjelenítése
 //*********************************
 ShowWindow(Form1, SajatiCmdShow);
 UpdateWindow(Form1);

 //*********************************
 //Ablak üzenetkezelésének aktiválása
 //*********************************
 while (GetMessage(&msg, NULL, 0, 0))
 {
  TranslateMessage(&msg);
  DispatchMessage(&msg);
 }
 return msg.wParam;
}

//*********************************
//Az ablak callback függvénye: eseménykezelés
//*********************************
LRESULT CALLBACK WndProc0(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
 HDC hdc;
 PAINTSTRUCT ps;
 FILE* myfile;
 unsigned int xPos, yPos, xPos2, yPos2, fwButtons;

 switch (message)
 {
  //*********************************
  //Ablak létrehozásakor közvetlenül
  //*********************************
 case WM_CREATE:
  /*Init*/;
  if ((joyGetNumDevs()) > 0) joySetCapture(hwnd, JOYSTICKID1, NULL, FALSE);
  fopen_s(&myfile, "CUDA_benchmark.txt", "wt");
  fclose(myfile);
  PEGA_benchmark_start();
  PEGA_init(hwnd);
  PEGA_init_3D();
  PEGA_benchmark_stop("----------->>>>init");

  PEGA_benchmark_start();
  PEGA_create_HOST_CUDA_3D_vertex_list(10000000);  

  PEGA_benchmark_stop("memory allocation");
  PEGA_drawing3D();
  PEGA_3D_vertex_reset();
  PEGA_obj_loader("3dmodel.obj");
  PEGA_log_dev_status(nyers_csucspontok_length,MAX_3d_vertex_count);
  PEGA_push_3D_triangles_to_GPU();
  return 0;
  //*********************************
  //Képernyővillogás elkerülése
  //*********************************
 case WM_ERASEBKGND:
  return (LRESULT)1;
 case MM_JOY1MOVE:
  fwButtons = wParam;
  xPos = LOWORD(lParam);
  yPos = HIWORD(lParam);
   if (xPos == 65535) {
    fszog2.y += 5.0; PEGA_drawing3D_update();
   }
   else if (xPos == 0) {
    fszog2.y -= 5.0; PEGA_drawing3D_update();
   }
   if (yPos == 65535) {
    fszog2.x += 5.0; PEGA_drawing3D_update();
   }
   else if (yPos == 0) {
    fszog2.x -= 5.0; PEGA_drawing3D_update();
   }
   if (fwButtons == 128) {
    fszog2.z += 5.0; PEGA_drawing3D_update();
   }
   else if (fwButtons == 64) {
    fszog2.z -= 5.0; PEGA_drawing3D_update();
   }
   if (fszog2.y > 359) {
    fszog2.y = 0; PEGA_drawing3D_update();
   }
   else if (fszog2.y < 0) {
    fszog2.y = 358; PEGA_drawing3D_update();
   }
   if (fszog2.x > 359) {
    fszog2.x = 0; PEGA_drawing3D_update();
   }
   else if (fszog2.x < 0) {
    fszog2.x = 358; PEGA_drawing3D_update();
   }
   if (fszog2.z > 359) {
    fszog2.z = 0; PEGA_drawing3D_update();
   }
   else if (fszog2.z < 0) {
    fszog2.z = 358; PEGA_drawing3D_update();
   }
  if (fwButtons == 2)
  {
   nagyitas_ertek *= 1.02;
   PEGA_zoom_in();
   PEGA_drawing3D_update();
  }
  else if (fwButtons == 4)
  {
   nagyitas_ertek /= 1.02;
   PEGA_zoom_out();
   PEGA_drawing3D_update();
  }
  break;
  //*********************************
  //Ablak kliens területének újrarajzolása
  //*********************************
 case WM_PAINT:
  hdc = BeginPaint(hwnd, &ps);
  EndPaint(hwnd, &ps);
  return 0;
  //*********************************
  //Ablak bezárása, erőforrások felszabadítása
  //*********************************
 case WM_CLOSE:
  PEGA_benchmark_start();
  PEGA_free_Direct2D();
  PEGA_free3D();
  PEGA_benchmark_stop("----------->>>>memory free up");
  DestroyWindow(hwnd);
  return 0;
  //*********************************
  //Ablak megsemmisítése
  //*********************************
 case WM_DESTROY:
  PostQuitMessage(0);
  return 0;
 }
 return DefWindowProc(hwnd, message, wParam, lParam);
}

void PEGA_drawing3D_update(void)
{
 char tempstr2[128], tempstr[128];

 PEGA_start_fps_benchmark();
 PEGA_benchmark_start();
 PEGA_clearscreen();
 PEGA_benchmark_stop("----------->>>>clearscreen");

 PEGA_benchmark_start();
 PEGA_rotate_3D();
 PEGA_benchmark_stop("3D rotation");

 PEGA_benchmark_start();
 PEGA_render_3D();
 PEGA_benchmark_stop("rendering");

 char hibauzenet[256];
 strcpy_s(hibauzenet, cudaGetErrorString(cudaGetLastError()));
 SetWindowTextA(Form1, hibauzenet);

 PEGA_benchmark_start();
 if (CUDA_DEVICE_COUNT > 1)
 {
  PEGA_merge_down_zbuffers();  
 }
 PEGA_swap_buffer();
 PEGA_benchmark_stop("swap buffer");
 PEGA_get_fps_benchmark();

 strcpy(tempstr2, "FPS: "); _itoa(fps_stat, tempstr, 10); strcat(tempstr2, tempstr);
 SetWindowTextA(Form1, tempstr2);
}
