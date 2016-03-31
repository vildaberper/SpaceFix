#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS
#endif

#include <thread>
#include <mutex>

#include "resource.h"
#include "SpaceFix.h"

UINT WM_TASKBAR = 0;
HWND Hwnd;
HMENU Hmenu;
NOTIFYICONDATA notifyIconData;
TCHAR szTIP[64] = TEXT("SpaceFix");
char szClassName[] = "SpaceFix";
TCHAR nInfo[64] = TEXT("Double press detected");

LRESULT CALLBACK windowProcedure(HWND, UINT, WPARAM, LPARAM);
void minimize();
void restore();
void initNotifyIconData();

void notify(){
	/*notifyIconData.uFlags = NIF_INFO;

	strncpy_s(notifyIconData.szInfo, nInfo, sizeof(nInfo));
	strncpy_s(notifyIconData.szInfoTitle, szTIP, sizeof(szTIP));

	notifyIconData.dwInfoFlags = NIF_INFO;
	Shell_NotifyIcon(NIM_MODIFY, &notifyIconData);*/
}

SpaceFix sf;

std::mutex qLock;
bool quit = false;

std::mutex pLock;
unsigned long prevented = 0;

void sfLoop(){
	while(true){
		if(sf.tick()){
			pLock.lock();
			prevented++;
			pLock.unlock();
			notify();
		}
		Sleep(15);
		qLock.lock();
		if(quit){
			qLock.unlock();
			break;
		}
		else{
			qLock.unlock();
		}
	}
}

int WINAPI WinMain(HINSTANCE hThisInstance, HINSTANCE hPrevInstance, LPSTR lpszArgument, int nCmdShow){
	MSG messages;
	WNDCLASSEX wincl;
	WM_TASKBAR = RegisterWindowMessageA("TaskbarCreated");

	wincl.hInstance = hThisInstance;
	wincl.lpszClassName = szClassName;
	wincl.lpfnWndProc = windowProcedure;
	wincl.style = CS_DBLCLKS;
	wincl.cbSize = sizeof(WNDCLASSEX);

	wincl.hIcon = LoadIcon(GetModuleHandle(NULL), MAKEINTRESOURCE(ICO1));
	wincl.hIconSm = LoadIcon(GetModuleHandle(NULL), MAKEINTRESOURCE(ICO1));
	wincl.hCursor = LoadCursor(NULL, IDC_ARROW);
	wincl.lpszMenuName = NULL;
	wincl.cbClsExtra = 0;
	wincl.cbWndExtra = 0;
	wincl.hbrBackground = (HBRUSH) (CreateSolidBrush(RGB(255, 255, 255)));
	if(!RegisterClassEx(&wincl)){
		return 0;
	}

	Hwnd = CreateWindowEx(
		0,
		szClassName,
		szClassName,
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		400,
		170,
		HWND_DESKTOP,
		NULL,
		hThisInstance,
		NULL
		);

	SetWindowLong(Hwnd, GWL_STYLE, WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU);

	initNotifyIconData();
	Shell_NotifyIcon(NIM_ADD, &notifyIconData);

	/*AllocConsole();
	freopen("CONOUT$", "w", stdout);*/

	std::thread loop(sfLoop);

	while(GetMessage(&messages, NULL, 0, 0)){
		TranslateMessage(&messages);
		DispatchMessage(&messages);
	}

	qLock.lock();
	quit = true;
	qLock.unlock();
	loop.join();

	return messages.wParam;
}

LRESULT CALLBACK windowProcedure(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam){
	if(message == WM_TASKBAR && !IsWindowVisible(Hwnd)){
		minimize();
		return 0;
	}

	switch(message){

	case WM_ACTIVATE:
		Shell_NotifyIcon(NIM_ADD, &notifyIconData);
		break;

	case WM_CREATE:
		ShowWindow(Hwnd, SW_HIDE);
		Hmenu = CreatePopupMenu();
		AppendMenu(Hmenu, MF_STRING, ID_TRAY_EXIT, TEXT("Exit"));
		break;

	case WM_SYSCOMMAND:
		switch(wParam & 0xFFF0){
		case SC_MINIMIZE:
		case SC_CLOSE:
			minimize();
			return 0;
			break;
		}
		break;

	case WM_SYSICON:
	{
		switch(wParam){
		case ID_TRAY_APP_ICON:
			//SetForegroundWindow(Hwnd);
			break;
		}
		if(lParam == WM_LBUTTONUP){
			restore();
			SetForegroundWindow(Hwnd);
		}
		else if(lParam == WM_RBUTTONDOWN){
			POINT curPoint;
			GetCursorPos(&curPoint);
			SetForegroundWindow(Hwnd);
			UINT clicked = TrackPopupMenu(Hmenu, TPM_RETURNCMD | TPM_NONOTIFY, curPoint.x, curPoint.y, 0, hwnd, NULL);
			SendMessage(hwnd, WM_NULL, 0, 0);
			if(clicked == ID_TRAY_EXIT){
				Shell_NotifyIcon(NIM_DELETE, &notifyIconData);
				PostQuitMessage(0);
			}
		}
	}
	break;

	case WM_NCHITTEST:
	{
		UINT uHitTest = DefWindowProc(hwnd, WM_NCHITTEST, wParam, lParam);
		if(uHitTest == HTCLIENT)
			return HTCAPTION;
		else
			return uHitTest;
	}

	case WM_CLOSE:
		minimize();
		return 0;
		break;

	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	}

	return DefWindowProc(hwnd, message, wParam, lParam);
}


void minimize(){
	ShowWindow(Hwnd, SW_HIDE);
}


void restore(){
	ShowWindow(Hwnd, SW_SHOW);
	HDC hdc = GetDC(Hwnd);
	RECT rect;
	GetClientRect(Hwnd, &rect);
	pLock.lock();
	std::string ts = "Accidents prevented: " + std::to_string(prevented) + "\n\nSpaceFix is a small program to fix accidental\ndouble presses on spacebar in selected applications.\n\nWritten by Oskar Viberg\nAKA vildaberper\nbumblebullet@gmail.com";
	LPCSTR text = ts.c_str();
	pLock.unlock();
	DrawTextA(hdc, text, strlen(text), &rect, DT_CENTER | DT_VCENTER);
}

void initNotifyIconData(){
	memset(&notifyIconData, 0, sizeof(NOTIFYICONDATA));
	notifyIconData.cbSize = sizeof(NOTIFYICONDATA);
	notifyIconData.hWnd = Hwnd;
	notifyIconData.uID = ID_TRAY_APP_ICON;
	notifyIconData.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
	notifyIconData.uCallbackMessage = WM_SYSICON;
	notifyIconData.hIcon = (HICON) LoadIcon(GetModuleHandle(NULL), MAKEINTRESOURCE(ICO1));
	strncpy_s(notifyIconData.szTip, szTIP, sizeof(szTIP));

	notifyIconData.uVersion = 0;
	Shell_NotifyIcon(NIM_SETVERSION, &notifyIconData);
}
