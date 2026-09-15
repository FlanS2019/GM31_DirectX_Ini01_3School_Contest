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

	// STEP39: 「FPS上限を無制限にすると異常に速くなる」バグの修正。
	// Manager::Update()やPlayer.cpp/Key.cpp/horror.cpp等、あちこちの
	// GameObjectが昔からdt=1/60を決め打ちで使っている(実時間を測っていない)。
	// STEP37までは常にUpdate()とDraw()をセットで「1000/60ms経ったら1回」
	// だけ呼んでいたので暗黙に60Hz相当になっていたが、FPS上限を「無制限」
	// にするとこのゲートが外れ、Draw()内部のPresent(1,0)がモニタの
	// リフレッシュレートに同期してブロックする関係で、144Hzのモニタでは
	// Update()も144回/秒近く呼ばれてしまい、dt=1/60前提のロジックが
	// その分(144/60=2.4倍)速く進んでしまっていた。
	//
	// 対策: Update()は「固定ステップの蓄積(アキュムレータ)」方式にして、
	// 実時間がどれだけ経っていても常に1000/60msぶんずつだけ進める
	// -- ゲームの進行速度がモニタのリフレッシュレートやFPS上限設定と
	// 完全に無関係になる(全GameObjectのdt=1/60前提はそのまま生かせるので、
	// 個々のファイルを直す必要が無い)。Draw()側だけがFPS上限設定
	// (30/60/無制限)に従う -- 無制限の場合はここでは待たず、Present(1,0)
	// 自体のVSync待ちに任せる(このユーザーの環境なら144Hzで頭打ちになる)。
	DWORD updateAccumulatorMs = 0;
	const DWORD kFixedStepMs = 1000 / 60;
	const DWORD kMaxAccumulatorMs = kFixedStepMs * 5; // 大きく遅延した場合の暴走防止(最大5フレーム分だけ追いつく)
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

			// ゲームロジックは常に1000/60msぶんずつ、実時間とは切り離して進める。
			while (updateAccumulatorMs >= kFixedStepMs)
			{
				Manager::Update();
				updateAccumulatorMs -= kFixedStepMs;
			}

			// 描画だけFPS上限設定(30/60/無制限)に従う。設定画面から実行中に
			// 変更されることもあるので、キャッシュせず毎回
			// GameSettings::GetFpsCap()を読みに行く(ただのint取得なので
			// コストは無視できる)。
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

