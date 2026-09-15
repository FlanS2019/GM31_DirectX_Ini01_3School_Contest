#include "main.h"
#include "manager.h"
#include "audio.h"
#include "gameSettings.h"
#include <thread>

const wchar_t* CLASS_NAME = L"AppClass";
const wchar_t* WINDOW_NAME = L"abandoned ruins,0:00"; // renamed from "abandoned hospital" -- theme changed since the available models don't read as hospital-specific

LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

HWND g_Window;

HWND GetWindow()
{
	return g_Window;
}	

int APIENTRY wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow)
{
	SetProcessDPIAware();

	WNDCLASSEXW wcex;
	{
		wcex.cbSize = sizeof(WNDCLASSEXW);
		wcex.style = 0;
		wcex.lpfnWndProc = WndProc;
		wcex.cbClsExtra = 0;
		wcex.cbWndExtra = 0;
		wcex.hInstance = hInstance;
		wcex.hIcon = nullptr;
		wcex.hCursor = LoadCursorW(nullptr, L"IDC_ARROW");
		wcex.hbrBackground = nullptr;
		wcex.lpszMenuName = nullptr;
		wcex.lpszClassName = CLASS_NAME;
		wcex.hIconSm = nullptr;

		RegisterClassExW(&wcex);


		RECT rc = { 0, 0, (LONG)SCREEN_WIDTH, (LONG)SCREEN_HEIGHT };
		AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, FALSE);

		g_Window = CreateWindowExW(0, CLASS_NAME, WINDOW_NAME, WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT,
			rc.right - rc.left, rc.bottom - rc.top, nullptr, nullptr, hInstance, nullptr);
	}

	CoInitializeEx(nullptr, COINITBASE_MULTITHREADED);

	Audio::InitMaster();

	Manager::Init();

	GameSettings::SetFullscreen(GameSettings::GetFullscreen());

	ShowWindow(g_Window, nCmdShow);
	UpdateWindow(g_Window);
	


	DWORD dwExecLastTime;
	DWORD dwCurrentTime;
	timeBeginPeriod(1);
	dwExecLastTime = timeGetTime();
	dwCurrentTime = 0;

	DWORD updateAccumulatorMs = 0;
	const DWORD kFixedStepMs = 1000 / 60;
	const DWORD kMaxAccumulatorMs = kFixedStepMs * 5;
	DWORD dwLastDrawTime = 0;

	MSG msg;
	while(1)
	{
        if(PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			if(msg.message == WM_QUIT)
			{
				break;
			}
			else
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
        }
		else
		{
			dwCurrentTime = timeGetTime();

			DWORD elapsedMs = dwCurrentTime - dwExecLastTime;
			dwExecLastTime = dwCurrentTime;

			updateAccumulatorMs += elapsedMs;
			if (updateAccumulatorMs > kMaxAccumulatorMs) updateAccumulatorMs = kMaxAccumulatorMs;

			while (updateAccumulatorMs >= kFixedStepMs)
			{
				Manager::Update();
				updateAccumulatorMs -= kFixedStepMs;
			}

			int fpsCap = GameSettings::GetFpsCap();
			DWORD drawIntervalMs = (fpsCap == 0) ? (1000 / 30) : (fpsCap == 1) ? (1000 / 60) : 0;

			if (drawIntervalMs == 0 || (dwCurrentTime - dwLastDrawTime) >= drawIntervalMs)
			{
				dwLastDrawTime = dwCurrentTime;
				Manager::Draw();
			}
		}
	}

	timeEndPeriod(1);

	UnregisterClassW(CLASS_NAME, wcex.hInstance);

	Manager::Uninit();
	
	Audio::UninitMaster();

	CoUninitialize();

	return (int)msg.wParam;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{

	switch(uMsg)
	{
	case WM_DESTROY:
		PostQuitMessage(0);
		break;


	default:
		break;
	}

	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

