#include "main.h"
#include "pauseMenu.h"
#include "settingsScreen.h"
#include "manager.h"
#include "Input.h"
#include "hud.h"
#include "soundManager.h"
#include "title.h"
#include "menuSound.h" // STEP27

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

	// STEP34: 1.5x -- DrawUI()とUpdate()(マウスの当たり判定)の両方から
	// 参照するので、ここに1箇所だけ置いて食い違いが起きないようにする。
	const float kPanelW = 630.0f;
	const float kPanelH = 480.0f;
	const float kRowH = 69.0f;
	const float kTitleGap = kRowH;      // Root: 見出しの後の間隔
	const float kQuestionGap = kRowH * 1.2f; // Confirm: 質問文の後の間隔
	const float kItemRowH = kRowH * 0.75f;

	float PanelX() { return (SCREEN_WIDTH - kPanelW) * 0.5f; }
	float PanelY() { return (SCREEN_HEIGHT - kPanelH) * 0.5f; }

	// STEP37: マウスクリックでのメニュー操作用の当たり判定の半幅。
	const float kHitHalfWidth = 260.0f;

	// STEP37注記: titleMenu.cppのHitTestRow()と同じ考え方 -- yは文字列の
	// 上端なので、[rowTopY-6, rowTopY-6+rowH)で隙間無く敷き詰める。
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
	Input::SetMouseCaptureEnabled(false); // STEP36: 一時停止中はマウスを解放
}

void PauseMenu::Close()
{
	m_State = State::Closed;
	m_Selected = 0;
	Manager::SetPaused(false);
	SoundManager::SetPauseAttenuation(1.0f);
	Input::SetMouseCaptureEnabled(true); // STEP36: ゲームに戻るので再キャプチャ
}

void PauseMenu::Update()
{
	SettingsScreen* settings = Manager::GetGameObject<SettingsScreen>();
	MenuSound* menuSound = Manager::GetGameObject<MenuSound>(); // STEP27

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

	// STEP37: マウスでのホバー選択/クリック決定(DrawUI()と同じ座標定数
	// を使って各項目の矩形を再現)。
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

	if (Input::GetKeyTrigger(VK_RETURN) || mouseConfirm) // STEP37: マウス決定もEnterと同じ扱い
	{
		if (menuSound) menuSound->PlayConfirm(); // STEP27

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
				Input::SetMouseCaptureEnabled(false); // STEP36
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

	const float panelX = PanelX();
	const float panelY = PanelY();
	Hud::DrawPanel(panelX, panelY, kPanelW, kPanelH);

	const float centerX = SCREEN_WIDTH * 0.5f;
	float y = panelY + 45.0f; // STEP34: 1.5x

	if (m_State == State::Root)
	{
		Hud::DrawText("一時停止", centerX, y, 42.0f, true); // STEP34: 1.5x
		y += kTitleGap;

		for (int i = 0; i < kRootItemCount; i++)
		{
			char buf[64];
			sprintf_s(buf, "%s%s", (i == m_Selected) ? "> " : "  ", kRootLabels[i]);
			Hud::DrawText(buf, centerX, y, 33.0f, true); // STEP34: 1.5x
			y += kItemRowH;
		}
	}
	else
	{
		const char* question = (m_State == State::ConfirmTitle)
			? "タイトルへ戻りますか?"
			: "ゲームを終了しますか?";

		Hud::DrawText(question, centerX, y, 36.0f, true); // STEP34: 1.5x
		y += kQuestionGap;

		const char* confirmLabels[kConfirmItemCount] = { "いいえ", "はい" };
		for (int i = 0; i < kConfirmItemCount; i++)
		{
			char buf[32];
			sprintf_s(buf, "%s%s", (i == m_Selected) ? "> " : "  ", confirmLabels[i]);
			Hud::DrawText(buf, centerX, y, 33.0f, true); // STEP34: 1.5x
			y += kItemRowH;
		}
	}
}
