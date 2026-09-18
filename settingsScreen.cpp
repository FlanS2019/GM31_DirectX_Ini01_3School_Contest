#include "main.h"
#include "settingsScreen.h"
#include "gameSettings.h"
#include "Input.h"
#include "hud.h"
#include "manager.h" // STEP27: Manager::GetGameObject<MenuSound>()に必要
#include "menuSound.h" // STEP27
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
		Row_Resolution, // STEP48
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
		"解像度", // STEP48
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
	const char* kResolutionLabels[5] = { "144p", "360p", "480p", "1080p", "4K" }; // STEP48

	// STEP34: 1.5x -- DrawUI()とUpdate()(マウスの当たり判定)の両方から
	// 参照するので、ここに1箇所だけ置いて食い違いが起きないようにする。
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

	// 左右キーでの増減。スライダー/切り替え/サイクルの行だけが対象
	// (デフォルトに戻す/戻るはEnterでのみ動くアクション行)。STEP37:
	// マウスクリックでの「行の値を進める」もこれを直接呼ぶ。
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
		case Row_Resolution:
		{
			int v = GameSettings::GetResolutionIndex() + direction;
			if (v < 0) v = 4;
			if (v > 4) v = 0;
			GameSettings::SetResolutionIndex(v);
			break;
		}
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
		case Row_Resolution:
			sprintf_s(outBuf, bufSize, "%s", kResolutionLabels[GameSettings::GetResolutionIndex()]);
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

	MenuSound* menuSound = Manager::GetGameObject<MenuSound>(); // STEP27

	if (Input::GetKeyTrigger(VK_ESCAPE))
	{
		Close();
		return;
	}

	// STEP37: マウスでのホバー選択/クリック操作。行全体(ラベル～値の帯)を
	// 当たり判定にし、クリックすると: デフォルトに戻す/戻る行なら
	// Enterと同じ確定処理、それ以外の調整可能な行なら右キー1回分だけ
	// 値を進める(スライダーをクリックで進めるのと同じ感覚)。
	bool mouseConfirm = false;
	{
		int mx = Input::GetMouseX();
		int my = Input::GetMouseY();
		float panelX = PanelX();
		float panelY = PanelY();
		float rowLeft = panelX;
		float rowRight = panelX + kPanelW;
		// STEP37注記: DrawUI()のyは各行の文字列の上端で、panelY+30+kHeaderGapが
		// 最初の行のyと一致する(見出し分はkHeaderGapに織り込み済み)。
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

	if (Input::GetKeyTrigger(VK_RETURN) || mouseConfirm) // STEP37: マウス決定も同じ扱い
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
	const float labelX = panelX + 45.0f; // STEP34: 1.5x
	const float valueX = panelX + kPanelW - 165.0f; // STEP34: 1.5x // 左寄せで描く分の余白(kFpsCapLabels等の最大幅を想定)

	float y = panelY + 30.0f; // STEP34: 1.5x
	Hud::DrawText("設定", centerX, y, 39.0f, true); // STEP34: 1.5x
	y += kHeaderGap;

	for (int row = 0; row < Row_Count; row++)
	{
		bool selected = (row == m_Selected);
		char labelBuf[64];
		sprintf_s(labelBuf, "%s%s", selected ? "> " : "  ", kRowLabels[row]);
		Hud::DrawText(labelBuf, labelX, y, 30.0f, false); // STEP34: 1.5x

		if (row != Row_ResetDefault && row != Row_Back)
		{
			char valueBuf[32];
			FormatValueText(row, valueBuf, sizeof(valueBuf));
			Hud::DrawText(valueBuf, valueX, y, 30.0f, false); // STEP34: 1.5x
		}

		y += kRowH;
	}
}
