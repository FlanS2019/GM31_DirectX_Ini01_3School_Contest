#include "main.h"
#include "settingsScreen.h"
#include "gameSettings.h"
#include "Input.h"
#include "hud.h"
#include <cstdio>

namespace
{
	// STEP24: 追加仕様書13項の表の並び順そのまま(音量2つ、画面3つ、
	// 操作2つ、その他1つ)+ 末尾に「デフォルトに戻す」「戻る」の2ボタン。
	enum RowId
	{
		Row_BgmVolume,
		Row_SeVolume,
		Row_Brightness,
		Row_Fullscreen,
		Row_FpsCap,
		Row_MouseSensitivity,
		Row_InvertY,
		Row_HorrorIntensity,
		Row_ResetDefault,
		Row_Back,
		Row_Count,
	};

	const char* kRowLabels[Row_Count] =
	{
		"BGM音量",
		"SE音量",
		"画面の光量",
		"ウィンドウ/フルスクリーン",
		"フレームレート上限",
		"マウス感度",
		"Y軸反転",
		"ホラー演出の強さ",
		"デフォルトに戻す",
		"戻る",
	};

	const float kVolumeStep = 0.05f;
	const float kBrightnessStep = 0.05f;
	const float kSensitivityStep = 0.25f;

	const char* kFpsCapLabels[3] = { "30", "60", "無制限" };
	const char* kHorrorLabels[3] = { "弱", "中", "強" };

	float ClampStep(float value, float step, float lo, float hi)
	{
		if (value < lo) value = lo;
		if (value > hi) value = hi;
		return value;
	}

	// 左右キーでの増減。スライダー/切り替え/サイクルの行だけが対象
	// (デフォルトに戻す/戻るはEnterでのみ動くアクション行)。
	void AdjustRow(int row, int direction)
	{
		switch (row)
		{
		case Row_BgmVolume:
			GameSettings::SetBgmVolume(ClampStep(GameSettings::GetBgmVolume() + direction * kVolumeStep, kVolumeStep, 0.0f, 1.0f));
			break;
		case Row_SeVolume:
			GameSettings::SetSeVolume(ClampStep(GameSettings::GetSeVolume() + direction * kVolumeStep, kVolumeStep, 0.0f, 1.0f));
			break;
		case Row_Brightness:
			GameSettings::SetBrightness(ClampStep(GameSettings::GetBrightness() + direction * kBrightnessStep, kBrightnessStep, 0.0f, 1.0f));
			break;
		case Row_Fullscreen:
			GameSettings::SetFullscreen(!GameSettings::GetFullscreen()); // トグルなので方向は無視
			break;
		case Row_FpsCap:
		{
			int v = GameSettings::GetFpsCap() + direction;
			if (v < 0) v = 2;
			if (v > 2) v = 0;
			GameSettings::SetFpsCap(v);
			break;
		}
		case Row_MouseSensitivity:
			GameSettings::SetMouseSensitivity(ClampStep(GameSettings::GetMouseSensitivity() + direction * kSensitivityStep, kSensitivityStep, 0.25f, 3.0f));
			break;
		case Row_InvertY:
			GameSettings::SetInvertY(!GameSettings::GetInvertY());
			break;
		case Row_HorrorIntensity:
		{
			int v = GameSettings::GetHorrorIntensity() + direction;
			if (v < 0) v = 2;
			if (v > 2) v = 0;
			GameSettings::SetHorrorIntensity(v);
			break;
		}
		default:
			break; // Row_ResetDefault / Row_Back は左右キーでは何もしない
		}
	}

	void FormatValueText(int row, char* outBuf, size_t bufSize)
	{
		switch (row)
		{
		case Row_BgmVolume:
			sprintf_s(outBuf, bufSize, "%d%%", (int)(GameSettings::GetBgmVolume() * 100.0f + 0.5f));
			break;
		case Row_SeVolume:
			sprintf_s(outBuf, bufSize, "%d%%", (int)(GameSettings::GetSeVolume() * 100.0f + 0.5f));
			break;
		case Row_Brightness:
			sprintf_s(outBuf, bufSize, "%d%%", (int)(GameSettings::GetBrightness() * 100.0f + 0.5f));
			break;
		case Row_Fullscreen:
			sprintf_s(outBuf, bufSize, "%s", GameSettings::GetFullscreen() ? "フルスクリーン" : "ウィンドウ");
			break;
		case Row_FpsCap:
			sprintf_s(outBuf, bufSize, "%s", kFpsCapLabels[GameSettings::GetFpsCap()]);
			break;
		case Row_MouseSensitivity:
			sprintf_s(outBuf, bufSize, "%.2fx", GameSettings::GetMouseSensitivity());
			break;
		case Row_InvertY:
			sprintf_s(outBuf, bufSize, "%s", GameSettings::GetInvertY() ? "ON" : "OFF");
			break;
		case Row_HorrorIntensity:
			sprintf_s(outBuf, bufSize, "%s", kHorrorLabels[GameSettings::GetHorrorIntensity()]);
			break;
		default:
			outBuf[0] = '\0';
			break;
		}
	}
}

void SettingsScreen::Update()
{
	if (!m_Open) return;

	if (Input::GetKeyTrigger(VK_ESCAPE))
	{
		Close();
		return;
	}

	if (Input::GetKeyTrigger('W') || Input::GetKeyTrigger(VK_UP))
	{
		m_Selected = (m_Selected + Row_Count - 1) % Row_Count;
	}
	else if (Input::GetKeyTrigger('S') || Input::GetKeyTrigger(VK_DOWN))
	{
		m_Selected = (m_Selected + 1) % Row_Count;
	}

	if (Input::GetKeyTrigger('A') || Input::GetKeyTrigger(VK_LEFT))
	{
		AdjustRow(m_Selected, -1);
	}
	else if (Input::GetKeyTrigger('D') || Input::GetKeyTrigger(VK_RIGHT))
	{
		AdjustRow(m_Selected, +1);
	}

	if (Input::GetKeyTrigger(VK_RETURN))
	{
		if (m_Selected == Row_ResetDefault)
		{
			GameSettings::ResetToDefault();
		}
		else if (m_Selected == Row_Back)
		{
			Close();
		}
	}
}

void SettingsScreen::DrawUI()
{
	if (!m_Open) return;

	Hud::DrawFilledRect(0.0f, 0.0f, (float)SCREEN_WIDTH, (float)SCREEN_HEIGHT, 0.0f, 0.0f, 0.0f, 0.55f);

	const float panelW = 560.0f;
	const float panelH = 60.0f + Row_Count * 40.0f;
	const float panelX = (SCREEN_WIDTH - panelW) * 0.5f;
	const float panelY = (SCREEN_HEIGHT - panelH) * 0.5f;
	Hud::DrawPanel(panelX, panelY, panelW, panelH);

	const float centerX = SCREEN_WIDTH * 0.5f;
	const float labelX = panelX + 30.0f;
	const float valueX = panelX + panelW - 110.0f; // 左寄せで描く分の余白(kFpsCapLabels等の最大幅を想定)

	float y = panelY + 20.0f;
	Hud::DrawText("設定", centerX, y, 26.0f, true);
	y += 44.0f;

	for (int row = 0; row < Row_Count; row++)
	{
		bool selected = (row == m_Selected);
		char labelBuf[64];
		sprintf_s(labelBuf, "%s%s", selected ? "> " : "  ", kRowLabels[row]);
		Hud::DrawText(labelBuf, labelX, y, 20.0f, false);

		if (row != Row_ResetDefault && row != Row_Back)
		{
			char valueBuf[32];
			FormatValueText(row, valueBuf, sizeof(valueBuf));
			Hud::DrawText(valueBuf, valueX, y, 20.0f, false);
		}

		y += 40.0f;
	}
}
