#include "main.h"
#include "gameSettings.h"
#include "soundManager.h"
#include "manager.h"
#include "light.h"
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
	int g_HorrorIntensity = 1; // 0=low, 1=mid, 2=high

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
	SetHorrorIntensity(1);
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
	// STEP24: 値の保持のみ -- 追加仕様書19項の通り、実際のウィンドウモード
	// 切り替えは未実装(スワップチェーンのフルスクリーン制御まで手を
	// 入れるのは別途検討)。
	g_Fullscreen = fullscreen;
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
