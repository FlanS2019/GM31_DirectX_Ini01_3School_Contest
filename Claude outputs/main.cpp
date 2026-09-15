#include "main.h"
#include "manager.h"
#include "audio.h"
#include "gameSettings.h" // STEP37: FPS上限/フルスクリーン設定を読むため
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
	// STEP33: マウスの視点操作が全く効かない件への対策。Input::Update()は
	// GetCursorPos()/SetCursorPos()とウィンドウのクライアント矩形を突き合わせて
	// 移動量を計算しているが、プロセスがDPI非対応のままだと、Windowsが
	// ウィンドウ座標系をスケーリングして渡してくる一方でGetCursorPos()は
	// 物理ピクセルを返すため、座標系がズレてマウス移動量がほぼ0に潰れて
	// しまうことがある(高DPI/拡大率100%以外のディスプレイで典型的な症状)。
	// ウィンドウ作成より前にDPI対応を明示しておくことで座標系を揃える。
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

	// STEP37: GameSettings::Load()(Manager::Init()の中、GameSettings::
	// Init()経由で呼ばれる)はg_Fullscreenへ直接読み込むだけで、実際の
	// ウィンドウへは反映しない(gameSettings.cppのSetFullscreen()コメント
	// 参照)。ここでSetFullscreen()を経由させて、起動時に設定ファイルの
	// 値(前回フルスクリーンで終了していた場合など)を実際のウィンドウへ
	// 反映させる。
	GameSettings::SetFullscreen(GameSettings::GetFullscreen());

	ShowWindow(g_Window, nCmdShow);
	UpdateWindow(g_Window);
	


	DWORD dwExecLastTime;
	DWORD dwCurrentTime;
	timeBeginPeriod(1);
	dwExecLastTime = timeGetTime();
	dwCurrentTime = 0;



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

			// STEP37: 追加仕様書のFPS上限設定を実際にここへ反映。
			// 0=30fps, 1=60fps, 2=無制限(待たずに毎回更新)。設定画面から
			// 実行中に変更されることもあるので、キャッシュせず毎回
			// GameSettings::GetFpsCap()を読みに行く(ただのint取得なので
			// コストは無視できる)。
			int fpsCap = GameSettings::GetFpsCap();
			DWORD frameIntervalMs = (fpsCap == 0) ? (1000 / 30) : (fpsCap == 1) ? (1000 / 60) : 0;

			if (frameIntervalMs == 0 || (dwCurrentTime - dwExecLastTime) >= frameIntervalMs)
			{
				dwExecLastTime = dwCurrentTime;

				Manager::Update();
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

	// STEP32: 以前はここでESCキー=即ウィンドウ破棄(=アプリ終了)にして
	// いたが、これだとゲームプレイ中にESCを押すとPauseMenuを開く前に
	// アプリごと終了してしまう(PauseMenu/TitleMenuのConfirmQuitが
	// 「ゲーム終了」の意思確認を担当するようになった今は不要かつ有害)。
	// ESCキー自体はInput::GetKeyTrigger(VK_ESCAPE)で各シーン側が
	// ポーリングして処理するので、ここでは何もしない。

	default:
		break;
	}

	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

