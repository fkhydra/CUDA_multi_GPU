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

//***********STANDARD WIN32API ABLAKKEZELES************
HINSTANCE hInstGlob;
int SajatiCmdShow;
HWND Form1; //Ablak kezeloje
LRESULT CALLBACK WndProc0(HWND, UINT, WPARAM, LPARAM);
//******************************************************

//************************
void PEGA_drawing2D(void);
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

 switch (message)
 {
  //*********************************
  //Ablak létrehozásakor közvetlenül
  //*********************************
 case WM_CREATE:
  /*Init*/;
  srand((unsigned)time(NULL));
  PEGA_init(hwnd);
  PEGA_create_HOST_2D_point_list(10000);
  PEGA_create_CUDA_2D_point_list(10000);
  PEGA_create_HOST_2D_line_list(10000);
  PEGA_create_CUDA_2D_line_list(10000);
  PEGA_create_HOST_2D_triangle_list(10000);
  PEGA_create_CUDA_2D_triangle_list(10000);
  return 0;
  //*********************************
  //Képernyővillogás elkerülése
  //*********************************
 case WM_ERASEBKGND:
  return (LRESULT)1;
  //*********************************
  //Ablak kliens területének újrarajzolása
  //*********************************
 case WM_PAINT:
  hdc = BeginPaint(hwnd, &ps);
  EndPaint(hwnd, &ps);
  PEGA_drawing2D();
  return 0;
  //*********************************
  //Ablak bezárása, erőforrások felszabadítása
  //*********************************
 case WM_CLOSE:
  PEGA_free_Direct2D();
  PEGA_free2D();
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

void PEGA_drawing2D(void)
{
 int i, j;
 PEGA_clearscreen();

 // pontrajzolás
 PEGA_2D_point_reset();
 for (j = 0; j < 5000; j += 1)
  PEGA_add_2D_point(get_rnd(1899), get_rnd(999), RGB(255, 0, 0));

 // vonalrajzolás
 PEGA_2D_line_reset();
 PEGA_add_2D_line(0, 0, 1900,1000,RGB(0, 0, 0));
 PEGA_add_2D_line(1900, 0, 0, 1000, RGB(0, 0, 0));
 for (j = 0; j < 1900; j += 100)
  PEGA_add_2D_line(j, 0, j, 1000, RGB(0, 0, 0));
 PEGA_add_2D_line(0, 2, 1898, 2, RGB(0, 0, 0));
 PEGA_add_2D_line(1898, 2, 1898, 998, RGB(0, 0, 0));
 PEGA_add_2D_line(0, 998, 0, 2, RGB(0, 0, 0));
 PEGA_add_2D_line(1898, 998, 0, 998, RGB(0, 0, 0));

 // háromszögrajzolás
 PEGA_2D_triangle_reset();
 PEGA_add_2D_triangle(get_rnd(1899), get_rnd(999), get_rnd(1899), get_rnd(999), get_rnd(1899), get_rnd(999), RGB(get_rnd(255), get_rnd(255), get_rnd(255)));

 PEGA_push_points_to_GPU();
 PEGA_push_lines_to_GPU();
 PEGA_push_triangles_to_GPU();  
 PEGA_render_2D(); 
 
 PEGA_merge_down_2D_buffer();
 PEGA_swap_buffer();

 char hibauzenet[256];
 strcpy_s(hibauzenet, cudaGetErrorString(cudaGetLastError()));
 SetWindowTextA(Form1, hibauzenet);
}
