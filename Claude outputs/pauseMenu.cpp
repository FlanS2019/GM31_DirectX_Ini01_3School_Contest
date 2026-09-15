#include "main.h"
#include "pauseMenu.h"
#include "settingsScreen.h"
#include "manager.h"
#include "Input.h"
#include "hud.h"
#include "soundManager.h"
#include "title.h"

namespace
{
	const int kRootItemCount = 4; // ゲームに戻る / 設定 / タイトルへ戻る / ゲーム終了
	const int kConfirmItemCount = 2; // いいえ / はい (誤操作防止のためデフォルトは「いいえ」側)

	// STEP24: ポーズ中はアンビエントSEの音量を落として鳴らし続ける
	// (追加仕様書15項)。BGMはこの対象外 -- SoundManager::
	// SetPauseAttenuation()のコメント参照。
	const float kPauseSeAttenuation = 0.5f;

	const char* kRootLabels[kRootItemCount] =
	{
		"ゲームに戻る",
		"設定",
		"タイトルへ戻る",
		"ゲーム終了",
	};
}

void PauseMenu::Open()
{
	m_State = State::Root;
	m_Selected = 0;
	Manager::SetPaused(true);
	SoundManager::SetPauseAttenuation(kPauseSeAttenuation);
}

void PauseMenu::Close()
{
	m_State = State::Closed;
	m_Selected = 0;
	Manager::SetPaused(false);
	SoundManager::SetPauseAttenuation(1.0f);
}

void PauseMenu::Update()
{
	SettingsScreen* settings = Manager::GetGameObject<SettingsScreen>();

	// 設定画面がこのポーズ経由で開いている間は、ESC/Enter等の入力は
	// 設定画面側(SettingsScreen::Update())が完全に持つ -- 同じキー入力を
	// PauseMenuの側でも処理してしまうと、例えば設定の「戻る」を選んだ
	// Enterがそのままルートメニューの選択としても解釈されてしまう。
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

	// --- ここから m_State != Closed ---

	if (Input::GetKeyTrigger(VK_ESCAPE))
	{
		if (m_State == State::Root)
		{
			Close(); // ルートでのESCはゲームに戻るのと同じ扱い(12項)
		}
		else
		{
			// 確認画面からのESCは1段階だけ閉じる(いきなりゲームへは戻らない)
			m_State = State::Root;
			m_Selected = 1; // 直前に選んでいたであろう項目付近に戻す
		}
		return;
	}

	int itemCount = (m_State == State::Root) ? kRootItemCount : kConfirmItemCount;

	if (Input::GetKeyTrigger('W') || Input::GetKeyTrigger(VK_UP))
	{
		m_Selected = (m_Selected + itemCount - 1) % itemCount;
	}
	else if (Input::GetKeyTrigger('S') || Input::GetKeyTrigger(VK_DOWN))
	{
		m_Selected = (m_Selected + 1) % itemCount;
	}

	if (Input::GetKeyTrigger(VK_RETURN))
	{
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
				m_Selected = 0; // 「いいえ」をデフォルト選択
				break;
			case 3: // ゲーム終了
				m_State = State::ConfirmQuit;
				m_Selected = 0; // 「いいえ」をデフォルト選択
				break;
			}
		}
		else if (m_State == State::ConfirmTitle)
		{
			if (m_Selected == 1) // はい
			{
				Manager::SetPaused(false);
				SoundManager::SetPauseAttenuation(1.0f);
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
		return; // 設定画面が自分のパネルを描く -- 二重に背景を重ねない
	}

	if (m_State == State::Closed) return;

	// 一時停止中のフリーズフレーム(既に描画済みの3Dシーン)の上に、
	// 半透明の黒いパネルを重ねる(追加仕様書12項)。
	Hud::DrawFilledRect(0.0f, 0.0f, (float)SCREEN_WIDTH, (float)SCREEN_HEIGHT, 0.0f, 0.0f, 0.0f, 0.55f);

	const float panelW = 420.0f;
	const float panelH = 320.0f;
	const float panelX = (SCREEN_WIDTH - panelW) * 0.5f;
	const float panelY = (SCREEN_HEIGHT - panelH) * 0.5f;
	Hud::DrawPanel(panelX, panelY, panelW, panelH);

	const float centerX = SCREEN_WIDTH * 0.5f;
	float y = panelY + 30.0f;
	const float rowH = 46.0f;

	if (m_State == State::Root)
	{
		Hud::DrawText("一時停止", centerX, y, 28.0f, true);
		y += rowH;

		for (int i = 0; i < kRootItemCount; i++)
		{
			char buf[64];
			sprintf_s(buf, "%s%s", (i == m_Selected) ? "> " : "  ", kRootLabels[i]);
			Hud::DrawText(buf, centerX, y, 22.0f, true);
			y += rowH * 0.75f;
		}
	}
	else
	{
		const char* question = (m_State == State::ConfirmTitle)
			? "タイトルへ戻りますか?"
			: "ゲームを終了しますか?";

		Hud::DrawText(question, centerX, y, 24.0f, true);
		y += rowH * 1.2f;

		const char* confirmLabels[kConfirmItemCount] = { "いいえ", "はい" };
		for (int i = 0; i < kConfirmItemCount; i++)
		{
			char buf[32];
			sprintf_s(buf, "%s%s", (i == m_Selected) ? "> " : "  ", confirmLabels[i]);
			Hud::DrawText(buf, centerX, y, 22.0f, true);
			y += rowH * 0.75f;
		}
	}
}
