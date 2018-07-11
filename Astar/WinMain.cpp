#include "Precompile.h"
#include "Struct_Define.h"
#include "Screen.h"

#define KEY_ONE 0x31
#define KEY_TWO 0x32
#define KEY_THREE 0x33
#define KEY_FOUR 0x34
#define KEY_FIVE 0x35

bool is_Lbutton_active = false;
SCREEN screen;

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpszCmdParam, int nCmdShow)
{
	HWND hWnd;
	HDC screen_DC;
	RECT screen_rect;
	MSG Message;

	LPCTSTR ClassName = __TEXT("WinAPI");
	WNDCLASS WndClass;

	WndClass.cbClsExtra = 0;
	WndClass.cbWndExtra = 0;
	WndClass.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
	WndClass.hCursor = LoadCursor(NULL, IDC_ARROW);
	WndClass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	WndClass.hInstance = hInstance;
	WndClass.lpfnWndProc = WndProc;
	WndClass.lpszClassName = ClassName;
	WndClass.lpszMenuName = NULL;
	WndClass.style = CS_HREDRAW | CS_VREDRAW;
	RegisterClass(&WndClass);

	hWnd = CreateWindow(ClassName, __TEXT("Title"), WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, NULL, (HMENU)NULL, hInstance, NULL);
	ShowWindow(hWnd, nCmdShow);

	screen_DC = GetDC(hWnd);
	GetClientRect(hWnd, &screen_rect);
	
	screen.SetBufferInfo(screen_DC, screen_rect);

	while (1)
	{
		if (PeekMessage(&Message, NULL, 0, 0, PM_REMOVE))
		{
			if (Message.message == WM_QUIT)
				break;

			TranslateMessage(&Message);
			DispatchMessage(&Message);
		}
		else
			screen.Run();
	}

	ReleaseDC(hWnd, screen_DC);
	return (int)Message.wParam;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam)
{
	switch (iMessage)
	{
		case WM_CREATE:
			return 0;

		case WM_KEYDOWN:
			switch (wParam)
			{
				case KEY_ONE:	screen.ChangeKey((BYTE)KEY_STATUS::paint_head);		break;
				case KEY_TWO:	screen.ChangeKey((BYTE)KEY_STATUS::paint_tail);		break;
				case KEY_THREE:	screen.ChangeKey((BYTE)KEY_STATUS::paint_wall);		break;
				case KEY_FOUR:	screen.ChangeKey((BYTE)KEY_STATUS::paint_empty);	break;
				case KEY_FIVE:	screen.ChangeKey((BYTE)KEY_STATUS::paint_clear);	break;
			}
			return 0;

		case WM_LBUTTONDOWN:
			is_Lbutton_active = true;
			screen.SetPosition(LOWORD(lParam), HIWORD(lParam));
			return 0;

		case WM_MOUSEMOVE:
			if (is_Lbutton_active == true)
				screen.SetPosition(LOWORD(lParam), HIWORD(lParam));
			return 0;

		case WM_LBUTTONUP:
			is_Lbutton_active = false;
			return 0;

		case WM_RBUTTONDOWN:
			screen.ChangeKey((BYTE)KEY_STATUS::start);
			return 0;

		case WM_DESTROY:
			PostQuitMessage(0);
			return 0;

		default:
			return (DefWindowProc(hWnd, iMessage, wParam, lParam));
	}
}