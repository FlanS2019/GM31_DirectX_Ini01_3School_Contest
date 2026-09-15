#include "main.h"
#include "settingsScreen.h"
#include "gameSettings.h"
#include "Input.h"
#include "hud.h"
#include "manager.h" 
#include "menuSound.h"
#include <cstdio>

namespace
{
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

	const float kPanelW = 840.0f;
	const float kPanelH = 90.0f + Row_Count * 60.0f;
	const float kHeaderGap = 66.0f; // 見出し「設定」の後、最初の行までの間隔
	const float kRowH = 60.0f;

	float PanelX() { return (SCREEN_WIDTH - kPanelW) * 0.5f; }
	float PanelY() { return (SCREEN_HEIGHT - kPanelH) * 0.5f; }

	float ClampStep(float value, float step, float lo, float hi)
	{
		if (value < lo) value = lo;
		if (value > hi) value = hi;
		return value;
	}

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
			GameSettings::SetFullscreen(!GameSettings::GetFullscreen()); 
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
			break;
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

	MenuSound* menuSound = Manager::GetGameObject<MenuSound>();

	if (Input::GetKeyTrigger(VK_ESCAPE))
	{
		Close();
		return;
	}

	bool mouseConfirm = false;
	{
		int mx = Input::GetMouseX();
		int my = Input::GetMouseY();
		float panelX = PanelX();
		float panelY = PanelY();
		float rowLeft = panelX;
		float rowRight = panelX + kPanelW;
		float rowY = panelY + 30.0f + kHeaderGap;

		for (int row = 0; row < Row_Count; row++)
		{
			float top = rowY - 6.0f;
			float bottom = top + kRowH;
			if (mx >= rowLeft && mx <= rowRight && my >= top && my <= bottom)
			{
				if (m_Selected != row)
				{
					m_Selected = row;
					if (menuSound) menuSound->PlayMove();
				}
				if (Input::GetMouseLeftTrigger())
				{
					if (row == Row_ResetDefault || row == Row_Back)
					{
						mouseConfirm = true;
					}
					else
					{
						AdjustRow(row, +1);
					}
				}
			}
			rowY += kRowH;
		}
	}

	if (Input::GetKeyTrigger('W') || Input::GetKeyTrigger(VK_UP))
	{
		m_Selected = (m_Selected + Row_Count - 1) % Row_Count;
		if (menuSound) menuSound->PlayMove();
	}
	else if (Input::GetKeyTrigger('S') || Input::GetKeyTrigger(VK_DOWN))
	{
		m_Selected = (m_Selected + 1) % Row_Count;
		if (menuSound) menuSound->PlayMove();
	}

	if (Input::GetKeyTrigger('A') || Input::GetKeyTrigger(VK_LEFT))
	{
		AdjustRow(m_Selected, -1);
	}
	else if (Input::GetKeyTrigger('D') || Input::GetKeyTrigger(VK_RIGHT))
	{
		AdjustRow(m_Selected, +1);
	}

	if (Input::GetKeyTrigger(VK_RETURN) || mouseConfirm)
	{
		if (m_Selected == Row_ResetDefault)
		{
			GameSettings::ResetToDefault();
			if (menuSound) menuSound->PlayConfirm();
		}
		else if (m_Selected == Row_Back)
		{
			Close();
			if (menuSound) menuSound->PlayConfirm();
		}
	}
}

void SettingsScreen::DrawUI()
{
	if (!m_Open) return;

	Hud::DrawFilledRect(0.0f, 0.0f, (float)SCREEN_WIDTH, (float)SCREEN_HEIGHT, 0.0f, 0.0f, 0.0f, 0.55f);

	const float panelX = PanelX();
	const float panelY = PanelY();
	Hud::DrawPanel(panelX, panelY, kPanelW, kPanelH);

	const float centerX = SCREEN_WIDTH * 0.5f;
	const float labelX = panelX + 45.0f; 
	const float valueX = panelX + kPanelW - 165.0f; 

	float y = panelY + 30.0f; 
	Hud::DrawText("設定", centerX, y, 39.0f, true); 
	y += kHeaderGap;

	for (int row = 0; row < Row_Count; row++)
	{
		bool selected = (row == m_Selected);
		char labelBuf[64];
		sprintf_s(labelBuf, "%s%s", selected ? "> " : "  ", kRowLabels[row]);
		Hud::DrawText(labelBuf, labelX, y, 30.0f, false);

		if (row != Row_ResetDefault && row != Row_Back)
		{
			char valueBuf[32];
			FormatValueText(row, valueBuf, sizeof(valueBuf));
			Hud::DrawText(valueBuf, valueX, y, 30.0f, false);
		}

		y += kRowH;
	}
}
