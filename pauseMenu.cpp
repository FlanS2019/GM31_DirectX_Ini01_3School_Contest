#include "main.h"
#include "pauseMenu.h"
#include "settingsScreen.h"
#include "manager.h"
#include "Input.h"
#include "hud.h"
#include "soundManager.h"
#include "title.h"
#include "menuSound.h" 

namespace
{
	const int kRootItemCount = 4;
	const int kConfirmItemCount = 2; 

	const float kPauseSeAttenuation = 0.5f;

	const char* kRootLabels[kRootItemCount] =
	{
		"ゲームに戻る",
		"設定",
		"タイトルへ戻る",
		"ゲーム終了",
	};

	const float kPanelW = 630.0f;
	const float kPanelH = 480.0f;
	const float kRowH = 69.0f;
	const float kTitleGap = kRowH;      // Root: 見出しの後の間隔
	const float kQuestionGap = kRowH * 1.2f; // Confirm: 質問文の後の間隔
	const float kItemRowH = kRowH * 0.75f;

	float PanelX() { return (SCREEN_WIDTH - kPanelW) * 0.5f; }
	float PanelY() { return (SCREEN_HEIGHT - kPanelH) * 0.5f; }

	const float kHitHalfWidth = 260.0f;

	bool HitTestRow(int mx, int my, float centerX, float rowTopY, float rowH)
	{
		float top = rowTopY - 6.0f;
		float bottom = top + rowH;
		return mx >= (centerX - kHitHalfWidth) && mx <= (centerX + kHitHalfWidth) && my >= top && my <= bottom;
	}
}

void PauseMenu::Open()
{
	m_State = State::Root;
	m_Selected = 0;
	Manager::SetPaused(true);
	SoundManager::SetPauseAttenuation(kPauseSeAttenuation);
	Input::SetMouseCaptureEnabled(false);
}

void PauseMenu::Close()
{
	m_State = State::Closed;
	m_Selected = 0;
	Manager::SetPaused(false);
	SoundManager::SetPauseAttenuation(1.0f);
	Input::SetMouseCaptureEnabled(true);
}

void PauseMenu::Update()
{
	SettingsScreen* settings = Manager::GetGameObject<SettingsScreen>();
	MenuSound* menuSound = Manager::GetGameObject<MenuSound>();

	if (settings && settings->IsOpen())
	{
		return;
	}

	if (m_State == State::Closed)
	{
		if (Input::GetKeyTrigger(VK_ESCAPE))
		{
			Open();
		}
		return;
	}


	if (Input::GetKeyTrigger(VK_ESCAPE))
	{
		if (m_State == State::Root)
		{
			Close();
		}
		else
		{
			m_State = State::Root;
			m_Selected = 1;
		}
		return;
	}

	int itemCount = (m_State == State::Root) ? kRootItemCount : kConfirmItemCount;

	bool mouseConfirm = false;
	{
		int mx = Input::GetMouseX();
		int my = Input::GetMouseY();
		float centerX = SCREEN_WIDTH * 0.5f;
		float rowY = PanelY() + 45.0f + ((m_State == State::Root) ? kTitleGap : kQuestionGap);
		float rowH = kItemRowH;

		for (int i = 0; i < itemCount; i++)
		{
			if (HitTestRow(mx, my, centerX, rowY, rowH))
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

	if (Input::GetKeyTrigger(VK_RETURN) || mouseConfirm)
	{
		if (menuSound) menuSound->PlayConfirm();

		if (m_State == State::Root)
		{
			switch (m_Selected)
			{
			case 0: // ゲームに戻る
				Close();
				break;
			case 1: // 設定
				if (settings) settings->Open();
				break;
			case 2: // タイトルへ戻る
				m_State = State::ConfirmTitle;
				m_Selected = 0;
				break;
			case 3: // ゲーム終了
				m_State = State::ConfirmQuit;
				m_Selected = 0; 
				break;
			}
		}
		else if (m_State == State::ConfirmTitle)
		{
			if (m_Selected == 1) // はい
			{
				Manager::SetPaused(false);
				SoundManager::SetPauseAttenuation(1.0f);
				Input::SetMouseCaptureEnabled(false);
				m_State = State::Closed;
				m_Selected = 0;
				Manager::ChangeScene<Title>(0.4f);
			}
			else
			{
				m_State = State::Root;
				m_Selected = 2;
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

void PauseMenu::DrawUI()
{
	SettingsScreen* settings = Manager::GetGameObject<SettingsScreen>();
	if (settings && settings->IsOpen())
	{
		return; 
	}

	if (m_State == State::Closed) return;

	Hud::DrawFilledRect(0.0f, 0.0f, (float)SCREEN_WIDTH, (float)SCREEN_HEIGHT, 0.0f, 0.0f, 0.0f, 0.55f);

	const float panelX = PanelX();
	const float panelY = PanelY();
	Hud::DrawPanel(panelX, panelY, kPanelW, kPanelH);

	const float centerX = SCREEN_WIDTH * 0.5f;
	float y = panelY + 45.0f; 

	if (m_State == State::Root)
	{
		Hud::DrawText("一時停止", centerX, y, 42.0f, true); 
		y += kTitleGap;

		for (int i = 0; i < kRootItemCount; i++)
		{
			char buf[64];
			sprintf_s(buf, "%s%s", (i == m_Selected) ? "> " : "  ", kRootLabels[i]);
			Hud::DrawText(buf, centerX, y, 33.0f, true);
			y += kItemRowH;
		}
	}
	else
	{
		const char* question = (m_State == State::ConfirmTitle)
			? "タイトルへ戻りますか?"
			: "ゲームを終了しますか?";

		Hud::DrawText(question, centerX, y, 36.0f, true);
		y += kQuestionGap;

		const char* confirmLabels[kConfirmItemCount] = { "いいえ", "はい" };
		for (int i = 0; i < kConfirmItemCount; i++)
		{
			char buf[32];
			sprintf_s(buf, "%s%s", (i == m_Selected) ? "> " : "  ", confirmLabels[i]);
			Hud::DrawText(buf, centerX, y, 33.0f, true);
			y += kItemRowH;
		}
	}
}
