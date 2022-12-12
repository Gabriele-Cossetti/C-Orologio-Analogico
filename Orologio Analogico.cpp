// HelloWindowsDesktop.cpp
// compile with: /D_UNICODE /DUNICODE /DWIN32 /D_WINDOWS /c
/*-----------------------------------------
   ANACLOCK.C -- Analog Clock Program Chap 07 Pag 305
                 (c) Charles Petzold, 1996
  -----------------------------------------*/
#pragma warning(disable : 4996)

#include <windows.h>
#include <time.h>
#include <math.h>
#include <tchar.h>

#define ID_TIMER    1
#define TWOPI       (2 * 3.14159)

static TCHAR szWindowClass[] = _T("DesktopApp");
static TCHAR szTitle[] = _T("Orologio analogico - Charles Petzold, 1996 - Chap 07 Pag 305");
HINSTANCE hInst;
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

int WINAPI WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPSTR lpCmdLine, _In_ int nCmdShow)
{
    static char szAppName[] = "AnaClock";
    HWND        hWnd;
    MSG         msg;

    WNDCLASSEX wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);
    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = WndProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInstance;
    wcex.hIcon = LoadIcon(wcex.hInstance, IDI_APPLICATION);
    wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wcex.lpszMenuName = NULL;
    wcex.lpszClassName = szWindowClass;
    wcex.hIconSm = LoadIcon(wcex.hInstance, IDI_APPLICATION);

    if (!RegisterClassEx(&wcex))
    {
        MessageBox(NULL, _T("Call to RegisterClassEx failed!"), _T("Windows Desktop Guided Tour"), NULL);
        return 1;
    }

    hInst = hInstance;
    hWnd = CreateWindowEx(WS_EX_OVERLAPPEDWINDOW, szWindowClass, szTitle, WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, 760, 660, NULL, NULL, hInstance, NULL);

    if (!SetTimer(hWnd, ID_TIMER, 1000, NULL))
    {
        MessageBox(hWnd, _T("Too many clocks or timers!"), szWindowClass,
            MB_ICONEXCLAMATION | MB_OK);
        return FALSE;
    }

    if (!hWnd)
    {
        MessageBox(NULL, _T("Call to CreateWindow failed!"), _T("Windows Desktop Guided Tour"), NULL);
        return 1;
    }

    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);

    while (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return (int)msg.wParam;
}

//------------------------------------ zona funzioni - inizio
void SetIsotropic(HDC hdc, int cxClient, int cyClient)   //modalità di mapping isotropa
{
    SetMapMode(hdc, MM_ISOTROPIC);
    SetWindowExtEx(hdc, 1000, 1000, NULL);
    SetViewportExtEx(hdc, cxClient / 2, -cyClient / 2, NULL);
    SetViewportOrgEx(hdc, cxClient / 2, cyClient / 2, NULL);
}

void RotatePoint(POINT pt[], int iNum, int iAngle)   //RotatePoint ruota le coordinate delle lancette
{
    int   i;
    POINT ptTemp;

    for (i = 0; i < iNum; i++)
    {
        ptTemp.x = (int)(pt[i].x * cos(TWOPI * iAngle / 360) +
            pt[i].y * sin(TWOPI * iAngle / 360));

        ptTemp.y = (int)(pt[i].y * cos(TWOPI * iAngle / 360) -
            pt[i].x * sin(TWOPI * iAngle / 360));

        pt[i] = ptTemp;
    }
}

void DrawClock(HDC hdc)      // DrawClock traccia i 60 punti sull'orologio
{
    int   iAngle;
    POINT pt[3];

    for (iAngle = 0; iAngle < 360; iAngle += 6)
    {
        pt[0].x = 0;
        pt[0].y = 900;

        RotatePoint(pt, 1, iAngle);

        pt[2].x = pt[2].y = iAngle % 5 ? 33 : 100;

        pt[0].x -= pt[2].x / 2;
        pt[0].y -= pt[2].y / 2;

        pt[1].x = pt[0].x + pt[2].x;
        pt[1].y = pt[0].y + pt[2].y;

        SelectObject(hdc, GetStockObject(BLACK_BRUSH));

        Ellipse(hdc, pt[0].x, pt[0].y, pt[1].x, pt[1].y);
    }
}

void DrawHands(HDC hdc, struct tm* datetime, BOOL bChange)   //DrawHands disegna le lancette
{
    static POINT pt[3][5] = { 0, -150, 100, 0, 0, 600, -100, 0, 0, -150,
                              0, -200,  50, 0, 0, 800,  -50, 0, 0, -200,
                              0,    0,   0, 0, 0,   0,    0, 0, 0,  800 }; //coordinate contorni lancette
    int          i, iAngle[3];
    POINT        ptTemp[3][5];

    iAngle[0] = (datetime->tm_hour * 30) % 360 + datetime->tm_min / 2;
    iAngle[1] = datetime->tm_min * 6;
    iAngle[2] = datetime->tm_sec * 6;

    memcpy(ptTemp, pt, sizeof(pt));

    for (i = bChange ? 0 : 2; i < 3; i++)
    {
        RotatePoint(ptTemp[i], 5, iAngle[i]);

        Polyline(hdc, ptTemp[i], 5);
    }
}
//------------------------------------ zona funzioni - fine

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    static int         cxClient, cyClient;
    static struct tm   dtPrevious;
    BOOL               bChange;
    HDC hdc;
    PAINTSTRUCT ps;
    struct tm datetime;
    time_t lTime;  

    switch (message)
    {
    case WM_CREATE:
        _time64(&lTime);
        datetime = *localtime(&lTime);

        dtPrevious = datetime;   //ottiene l'ora corrente e la memorizza in dtPrevious
        break;

    case WM_SIZE:
        cxClient = LOWORD(lParam);
        cyClient = HIWORD(lParam);
        break;

    case WM_TIMER:
        _time64(&lTime);
        datetime = *localtime(&lTime);

        bChange = datetime.tm_hour != dtPrevious.tm_hour || 
                  datetime.tm_min != dtPrevious.tm_min;

        hdc = GetDC(hWnd);

        SetIsotropic(hdc, cxClient, cyClient);

        SelectObject(hdc, GetStockObject(WHITE_PEN));
        DrawHands(hdc, &dtPrevious, bChange);

        SelectObject(hdc, GetStockObject(BLACK_PEN));
        DrawHands(hdc, &datetime, TRUE);

        ReleaseDC(hWnd, hdc);

        dtPrevious = datetime;
        break;

    case WM_PAINT:
        hdc = BeginPaint(hWnd, &ps);

        SetIsotropic(hdc, cxClient, cyClient);
        DrawClock(hdc);
        DrawHands(hdc, &dtPrevious, TRUE);

        EndPaint(hWnd, &ps);
        break;

    case WM_DESTROY:
        KillTimer(hWnd, ID_TIMER);
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
        break;
    }
    return 0;
}
