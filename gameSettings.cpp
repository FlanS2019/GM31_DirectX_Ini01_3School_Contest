#include "main.h"
#include "gameSettings.h"
#include "soundManager.h"
#include "manager.h"
#include "light.h"
#include "renderer.h" // STEP48: SetResolutionIndex() pushes into Renderer
#include <cstdio>
#include <cstring>
#include <algorithm>

namespace
{
	const char* kSettingsFile = "settings.ini";

	float g_BgmVolume = 1.0f;
	float g_SeVolume = 1.0f;
	float g_Brightness = 0.5f;
	bool g_Fullscreen = false;
	int g_FpsCap = 1; // 0=30, 1=60, 2=unlimited
	float g_MouseSensitivity = 1.0f;
	bool g_InvertY = false;
	int g_HorrorIntensity = 2; // 0=low, 1=mid, 2=high -- STEP27: 既定値を「強」に(大和さん指定: 「かなり怖くしておけ」)
	int g_ResolutionIndex = 3; // 0=144p,1=360p,2=480p,3=1080p,4=4K -- default 3 = native 1080p

	float Clamp01(float v) { return std::max(0.0f, std::min(1.0f, v)); }
}

void GameSettings::Init()
{
	Load(); // ファイルが無ければ中で何もしないので、既定値のまま進む

	// 起動直後、まだLightは存在しない(Game::Init()より前)ことが多いので
	// ここでLightへ反映する必要はない -- Light::Init()自身がGameSettings::
	// GetBrightness()を読みに行く(light.cpp参照)。SoundManagerは登録される
	// Audioがまだ無いので同様に何もしなくてよい。
}

void GameSettings::Load()
{
	FILE* file = nullptr;
	fopen_s(&file, kSettingsFile, "r");
	if (!file) return; // 初回起動などファイルが無い場合は既定値のまま

	char key[64];
	float fValue;
	char line[256];
	while (fgets(line, sizeof(line), file))
	{
		if (sscanf_s(line, "%63[^=]=%f", key, (unsigned)_countof(key), &fValue) != 2)
			continue;

		if (_stricmp(key, "BgmVolume") == 0) g_BgmVolume = Clamp01(fValue);
		else if (_stricmp(key, "SeVolume") == 0) g_SeVolume = Clamp01(fValue);
		else if (_stricmp(key, "Brightness") == 0) g_Brightness = Clamp01(fValue);
		else if (_stricmp(key, "Fullscreen") == 0) g_Fullscreen = (fValue != 0.0f);
		else if (_stricmp(key, "FpsCap") == 0) g_FpsCap = std::max(0, std::min(2, (int)fValue));
		else if (_stricmp(key, "MouseSensitivity") == 0) g_MouseSensitivity = std::max(0.25f, std::min(3.0f, fValue));
		else if (_stricmp(key, "InvertY") == 0) g_InvertY = (fValue != 0.0f);
		else if (_stricmp(key, "HorrorIntensity") == 0) g_HorrorIntensity = std::max(0, std::min(2, (int)fValue));
		else if (_stricmp(key, "Resolution") == 0) g_ResolutionIndex = std::max(0, std::min(4, (int)fValue));
	}

	fclose(file);
}

void GameSettings::Save()
{
	FILE* file = nullptr;
	fopen_s(&file, kSettingsFile, "w");
	if (!file) return; // 保存に失敗してもゲーム自体は問題なく続けられるので握りつぶす

	fprintf(file, "BgmVolume=%.2f\n", g_BgmVolume);
	fprintf(file, "SeVolume=%.2f\n", g_SeVolume);
	fprintf(file, "Brightness=%.2f\n", g_Brightness);
	fprintf(file, "Fullscreen=%d\n", g_Fullscreen ? 1 : 0);
	fprintf(file, "FpsCap=%d\n", g_FpsCap);
	fprintf(file, "MouseSensitivity=%.2f\n", g_MouseSensitivity);
	fprintf(file, "InvertY=%d\n", g_InvertY ? 1 : 0);
	fprintf(file, "HorrorIntensity=%d\n", g_HorrorIntensity);
	fprintf(file, "Resolution=%d\n", g_ResolutionIndex);

	fclose(file);
}

void GameSettings::ResetToDefault()
{
	SetBgmVolume(1.0f);
	SetSeVolume(1.0f);
	SetBrightness(0.5f);
	SetFullscreen(false);
	SetFpsCap(1);
	SetMouseSensitivity(1.0f);
	SetInvertY(false);
	SetHorrorIntensity(2); // STEP27: デフォルトに戻しても「強」のまま
	SetResolutionIndex(3); // STEP48: back to native 1080p, no upscaling
}

float GameSettings::GetBgmVolume() { return g_BgmVolume; }
void GameSettings::SetBgmVolume(float volume01)
{
	g_BgmVolume = Clamp01(volume01);
	SoundManager::ApplyVolumes();
}

float GameSettings::GetSeVolume() { return g_SeVolume; }
void GameSettings::SetSeVolume(float volume01)
{
	g_SeVolume = Clamp01(volume01);
	SoundManager::ApplyVolumes();
}

float GameSettings::GetBrightness() { return g_Brightness; }
void GameSettings::SetBrightness(float brightness01)
{
	g_Brightness = Clamp01(brightness01);

	Light* light = Manager::GetGameObject<Light>();
	if (light) light->SetBrightness01(g_Brightness);
}

bool GameSettings::GetFullscreen() { return g_Fullscreen; }
void GameSettings::SetFullscreen(bool fullscreen)
{
	g_Fullscreen = fullscreen;

	// STEP37: 追加仕様書19項で「時間があれば検討」とされていた実際の
	// ウィンドウモード切り替えを実装。スワップチェーンのフルスクリーン
	// (DXGIのSetFullscreenState)には手を入れず、ウィンドウ自体を
	// ボーダレスでプライマリモニタ全域まで広げる/元のウィンドウ枠に
	// 戻す、という軽量な方式にしてある。
	//
	// GameSettings::Load()はManager::Init()の中、main.cppがg_Windowを
	// 作成した後に呼ばれるので、通常はGetWindow()が有効なウィンドウを
	// 返す。ただしLoad()自身はこの関数を経由せずg_Fullscreenへ直接
	// 書き込むので、main.cpp側でManager::Init()の直後に一度
	// SetFullscreen(GetFullscreen())を呼び直し、設定ファイルから読んだ
	// 値を実際のウィンドウへ反映させている(main.cpp参照)。
	HWND hwnd = GetWindow();
	if (!hwnd) return; // ウィンドウ作成前(あり得ないはずだが念のため)

	// STEP38: 「フルスクリーンを切り替えるとウィンドウが最小化されたまま
	// 固定されてプレイ不能になる」バグの修正。原因はここの
	// else側(ウィンドウに戻す方)で、SetWindowPos()のX/Y引数に
	// CW_USEDEFAULTを渡していたこと -- CW_USEDEFAULTは
	// CreateWindow(Ex)専用の特殊値(実体はINT_MIN付近の巨大な負数)で、
	// SetWindowPos()はこれを「本物の座標」としてそのまま解釈してしまう。
	// 結果、ウィンドウが画面のはるか外(x,y = 約-21億)へ飛ばされて
	// 実質操作不能になり、それが「最小化されたまま」のように見えていた。
	// 対策: 実際の座標(プライマリモニタ中央)を計算して渡す。あわせて
	// 保険としてShowWindow(SW_SHOWNORMAL)も呼び、万一ウィンドウが
	// 最小化/非表示フラグを持っていても切り替え時に必ず通常表示へ戻す。
	if (fullscreen)
	{
		SetWindowLongPtr(hwnd, GWL_STYLE, WS_POPUP | WS_VISIBLE);
		ShowWindow(hwnd, SW_SHOWNORMAL);

		int screenW = GetSystemMetrics(SM_CXSCREEN);
		int screenH = GetSystemMetrics(SM_CYSCREEN);
		SetWindowPos(hwnd, HWND_TOP, 0, 0, screenW, screenH, SWP_FRAMECHANGED | SWP_NOZORDER);
	}
	else
	{
		SetWindowLongPtr(hwnd, GWL_STYLE, WS_OVERLAPPEDWINDOW | WS_VISIBLE);
		ShowWindow(hwnd, SW_SHOWNORMAL);

		RECT rc = { 0, 0, (LONG)SCREEN_WIDTH, (LONG)SCREEN_HEIGHT };
		AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, FALSE);
		int winW = rc.right - rc.left;
		int winH = rc.bottom - rc.top;

		// STEP38: CW_USEDEFAULTの代わりに、プライマリモニタ中央へ実座標で配置。
		int screenW = GetSystemMetrics(SM_CXSCREEN);
		int screenH = GetSystemMetrics(SM_CYSCREEN);
		int posX = (screenW - winW) / 2;
		int posY = (screenH - winH) / 2;

		SetWindowPos(hwnd, HWND_TOP, posX, posY, winW, winH, SWP_FRAMECHANGED | SWP_NOZORDER);
	}
}

int GameSettings::GetFpsCap() { return g_FpsCap; }
void GameSettings::SetFpsCap(int fpsCap)
{
	// STEP24: 値の保持のみ -- 実フレームレート制限(WinMain側)は未実装。
	g_FpsCap = std::max(0, std::min(2, fpsCap));
}

float GameSettings::GetMouseSensitivity() { return g_MouseSensitivity; }
void GameSettings::SetMouseSensitivity(float sensitivity)
{
	g_MouseSensitivity = std::max(0.25f, std::min(3.0f, sensitivity));
}

bool GameSettings::GetInvertY() { return g_InvertY; }
void GameSettings::SetInvertY(bool invert) { g_InvertY = invert; }

int GameSettings::GetHorrorIntensity() { return g_HorrorIntensity; }
void GameSettings::SetHorrorIntensity(int intensity)
{
	g_HorrorIntensity = std::max(0, std::min(2, intensity));
}

int GameSettings::GetResolutionIndex() { return g_ResolutionIndex; }
void GameSettings::SetResolutionIndex(int index)
{
	g_ResolutionIndex = std::max(0, std::min(4, index));
	Renderer::SetInternalResolution(g_ResolutionIndex);
}
