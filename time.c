#define UNICODE
#define _UNICODE
#include <windows.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include <sysinfoapi.h>

bool quit = false;
int ticks;
RECT updateArea;



LRESULT CALLBACK WindowProcessMessage(HWND, UINT, WPARAM, LPARAM);

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR pCmdLine, int nCmdShow) {
    WNDCLASS window_class = { 0 };
    const wchar_t window_class_name[] = L"My Window Class";
    window_class.lpszClassName = window_class_name;
    window_class.lpfnWndProc = WindowProcessMessage;
    window_class.hInstance = hInstance;
	window_class.hCursor = LoadCursor (NULL, IDC_ARROW);
    
    RegisterClass(&window_class);
    
    HWND window_handle = CreateWindow(window_class_name, L"Learn to Program Windows", WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, NULL, NULL, hInstance, NULL);
    if(window_handle == NULL) { return -1; }
    		updateArea.left = 0;
		updateArea.top = 0;
		updateArea.right = 150;
		updateArea.bottom = 70;
    ShowWindow(window_handle, nCmdShow);


    while(!quit) {
        MSG message;
        while(PeekMessage(&message, NULL, 0, 0, PM_REMOVE)) {
            TranslateMessage(&message);
            DispatchMessage(&message);
        }
		if(GetTickCount()%100 == 0){
			ticks++;
			InvalidateRect(window_handle, NULL, FALSE);
			UpdateWindow(window_handle);
		}
        // Do game stuff here
    }
    
    return 0;
}

LRESULT CALLBACK WindowProcessMessage(HWND window_handle, UINT message, WPARAM wParam, LPARAM lParam) {
	static PAINTSTRUCT paint;
	static HDC device_context;
    switch(message) {
        case WM_QUIT:
        case WM_DESTROY: {
            quit = true;
        } break;
	
	case WM_CREATE:{
		ticks = 0;

	}break;

	case WM_PAINT:{
		char buffer[50];
		device_context = BeginPaint(window_handle, &paint);
		FillRect(device_context, &paint.rcPaint, GetStockObject(GRAY_BRUSH));
		sprintf(buffer, "ticks: %d", ticks);
		TextOutA(device_context, 50, 50, buffer, 50);
		EndPaint(window_handle, &paint);
	}break;
        
        default: { // Message not handled; pass on to default message handling function
            return DefWindowProc(window_handle, message, wParam, lParam);
        } break;
    }
    return 0;
}