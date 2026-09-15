#include "main.h"
#include "titleMenu.h"
#include "settingsScreen.h"
#include "manager.h"
#include "Input.h"
#include "hud.h"
#include "Game.h"
#include "howToPlay.h"
#include "menuSound.h" // STEP27
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

	// STEP34: 1.5x -- Draw()とUpdate()(マウスの当たり判定)の両方から
	// 参照するので、ここに1箇所だけ置いて食い違いが起きないようにする。
	const float kCenterX = SCREEN_WIDTH * 0.5f;
	const float kRootStartY = SCREEN_HEIGHT * 0.55f;
	const float kRowH = 69.0f;
	const float kConfirmGap = kRowH * 1.2f;
	const float kConfirmRowH = kRowH * 0.75f;

	// STEP37: マウスクリックでのメニュー操作用の当たり判定の半幅
	// (中央揃えテキストの左右に余裕を持たせてある)。
	const float kHitHalfWidth = 320.0f;

	// カーソル位置(mx,my)がcenterXを中心としたこの行の矩形内にあるか。
	// STEP37注記: Hud::DrawText()のyは文字列の「上端」(hud.cppのpanel計算
	// 参照、中央ではない)。rowTopYはDraw()でそのままDrawText()に渡す
	// yの値と同じものを渡す想定 -- 各行はrowHずつ積み上がるので、
	// [rowTopY-6, rowTopY-6+rowH) で隙間無く敷き詰められる。
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
	MenuSound* menuSound = Manager::GetGameObject<MenuSound>(); // STEP27

	// 設定画面が開いている間はそちらが入力を持つ(pauseMenu.cppと同じ考え方)。
	if (settings && settings->IsOpen())
	{
		return;
	}

	int itemCount = (m_State == State::Root) ? kRootItemCount : kConfirmItemCount;

	// STEP37: マウスでのホバー選択/クリック決定。Draw()と同じ座標定数
	// (上のnamespace)を使って各項目の矩形を再現し、カーソルが乗っている
	// 項目にm_Selectedを合わせる(ホバー時に選択音も鳴らす)。クリックは
	// 下のEnter判定と合流させ、同じ確定処理を通す。
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

	// 確認画面からのESCは1段階だけ閉じる(pauseMenu.cppと同じ挙動)。
	if (m_State == State::ConfirmQuit && Input::GetKeyTrigger(VK_ESCAPE))
	{
		m_State = State::Root;
		m_Selected = 3; // 「ゲーム終了」に戻す
		return;
	}

	if (Input::GetKeyTrigger(VK_RETURN) || mouseConfirm) // STEP37: マウス決定もEnterと同じ扱い
	{
		if (menuSound) menuSound->PlayConfirm(); // STEP27

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

	// Titleシーンには他にHud::Begin()/End()を使うオブジェクトが無いので、
	// interact.cppのような外部からの間接呼び出しは不要 -- ここで直接開閉する。
	Hud::Begin();

	float y = kRootStartY;

	if (m_State == State::Root)
	{
		for (int i = 0; i < kRootItemCount; i++)
		{
			char buf[32];
			sprintf_s(buf, "%s%s", (i == m_Selected) ? "> " : "  ", kRootLabels[i]);
			Hud::DrawText(buf, kCenterX, y, 36.0f, true); // STEP34: 1.5x
			y += kRowH;
		}
	}
	else
	{
		Hud::DrawText("ゲームを終了しますか?", kCenterX, y, 36.0f, true); // STEP34: 1.5x
		y += kConfirmGap;

		const char* confirmLabels[kConfirmItemCount] = { "いいえ", "はい" };
		for (int i = 0; i < kConfirmItemCount; i++)
		{
			char buf[32];
			sprintf_s(buf, "%s%s", (i == m_Selected) ? "> " : "  ", confirmLabels[i]);
			Hud::DrawText(buf, kCenterX, y, 33.0f, true); // STEP34: 1.5x
			y += kConfirmRowH;
		}
	}

	if (settings) settings->DrawUI();

	// STEP37: シーン遷移の黒フェード。最前面(=このBegin/Endの一番最後)に
	// 重ねる -- Manager::GetFadeAlpha()が0の間は何も描かない(DrawFullScreenTint
	// 側でa<=0なら早期リターンする)ので、通常時のコストはほぼ無い。
	Hud::DrawFullScreenTint(0.0f, 0.0f, 0.0f, Manager::GetFadeAlpha());

	Hud::End();
}
