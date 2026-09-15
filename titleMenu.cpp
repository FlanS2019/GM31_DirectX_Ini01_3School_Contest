#include "main.h"
#include "titleMenu.h"
#include "settingsScreen.h"
#include "manager.h"
#include "Input.h"
#include "hud.h"
#include "Game.h"
#include "howToPlay.h"
#include "menuSound.h" 
#include <cstdio>

namespace
{
	const int kRootItemCount = 4; // ゲームスタート / 操作説明 / 設定 / ゲーム終了
	const int kConfirmItemCount = 2; // いいえ / はい(誤操作防止のためデフォルトは「いいえ」側)

	const char* kRootLabels[kRootItemCount] =
	{
		"ゲームスタート",
		"操作説明",
		"設定",
		"ゲーム終了",
	};

	const float kCenterX = SCREEN_WIDTH * 0.5f;
	const float kRootStartY = SCREEN_HEIGHT * 0.55f;
	const float kRowH = 69.0f;
	const float kConfirmGap = kRowH * 1.2f;
	const float kConfirmRowH = kRowH * 0.75f;

	const float kHitHalfWidth = 320.0f;

	bool HitTestRow(int mx, int my, float rowTopY, float rowH)
	{
		float top = rowTopY - 6.0f;
		float bottom = top + rowH;
		return mx >= (kCenterX - kHitHalfWidth) && mx <= (kCenterX + kHitHalfWidth) && my >= top && my <= bottom;
	}
}

void TitleMenu::Update()
{
	SettingsScreen* settings = Manager::GetGameObject<SettingsScreen>();
	MenuSound* menuSound = Manager::GetGameObject<MenuSound>(); 

	if (settings && settings->IsOpen())
	{
		return;
	}

	int itemCount = (m_State == State::Root) ? kRootItemCount : kConfirmItemCount;

	bool mouseConfirm = false;
	{
		int mx = Input::GetMouseX();
		int my = Input::GetMouseY();
		float rowY = (m_State == State::Root) ? kRootStartY : (kRootStartY + kConfirmGap);
		float rowH = (m_State == State::Root) ? kRowH : kConfirmRowH;

		for (int i = 0; i < itemCount; i++)
		{
			if (HitTestRow(mx, my, rowY, rowH))
			{
				if (m_Selected != i)
				{
					m_Selected = i;
					if (menuSound) menuSound->PlayMove();
				}
				if (Input::GetMouseLeftTrigger())
				{
					mouseConfirm = true;
				}
			}
			rowY += rowH;
		}
	}

	if (Input::GetKeyTrigger('W') || Input::GetKeyTrigger(VK_UP))
	{
		m_Selected = (m_Selected + itemCount - 1) % itemCount;
		if (menuSound) menuSound->PlayMove();
	}
	else if (Input::GetKeyTrigger('S') || Input::GetKeyTrigger(VK_DOWN))
	{
		m_Selected = (m_Selected + 1) % itemCount;
		if (menuSound) menuSound->PlayMove();
	}

	if (m_State == State::ConfirmQuit && Input::GetKeyTrigger(VK_ESCAPE))
	{
		m_State = State::Root;
		m_Selected = 3; // 「ゲーム終了」に戻す
		return;
	}

	if (Input::GetKeyTrigger(VK_RETURN) || mouseConfirm)
	{
		if (menuSound) menuSound->PlayConfirm();

		if (m_State == State::Root)
		{
			switch (m_Selected)
			{
			case 0: // ゲームスタート
				Manager::ChangeScene<Game>();
				break;
			case 1: // 操作説明
				Manager::ChangeScene<HowToPlay>();
				break;
			case 2: // 設定
				if (settings) settings->Open();
				break;
			case 3: // ゲーム終了
				m_State = State::ConfirmQuit;
				m_Selected = 0; // 「いいえ」をデフォルト選択
				break;
			}
		}
		else if (m_State == State::ConfirmQuit)
		{
			if (m_Selected == 1) // はい
			{
				PostQuitMessage(0);
			}
			else
			{
				m_State = State::Root;
				m_Selected = 3;
			}
		}
	}
}

void TitleMenu::Draw()
{
	SettingsScreen* settings = Manager::GetGameObject<SettingsScreen>();

	Hud::Begin();

	float y = kRootStartY;

	if (m_State == State::Root)
	{
		for (int i = 0; i < kRootItemCount; i++)
		{
			char buf[32];
			sprintf_s(buf, "%s%s", (i == m_Selected) ? "> " : "  ", kRootLabels[i]);
			Hud::DrawText(buf, kCenterX, y, 36.0f, true);
			y += kRowH;
		}
	}
	else
	{
		Hud::DrawText("ゲームを終了しますか?", kCenterX, y, 36.0f, true);
		y += kConfirmGap;

		const char* confirmLabels[kConfirmItemCount] = { "いいえ", "はい" };
		for (int i = 0; i < kConfirmItemCount; i++)
		{
			char buf[32];
			sprintf_s(buf, "%s%s", (i == m_Selected) ? "> " : "  ", confirmLabels[i]);
			Hud::DrawText(buf, kCenterX, y, 33.0f, true);
			y += kConfirmRowH;
		}
	}

	if (settings) settings->DrawUI();

	Hud::DrawFullScreenTint(0.0f, 0.0f, 0.0f, Manager::GetFadeAlpha());

	Hud::End();
}
