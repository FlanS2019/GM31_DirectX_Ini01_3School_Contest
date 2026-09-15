#include "main.h"
#include "howToPlayMenu.h"
#include "manager.h"
#include "Input.h"
#include "hud.h"
#include "title.h"
#include "menuSound.h" // STEP27
#include <cstdio>

namespace
{
	// 現状は「戻る」のみ。将来ここに項目を足すこともできる
	// (TitleMenu/PauseMenuと同じ配列パターン)。
	const int kItemCount = 1;
	const char* kLabels[kItemCount] = { "戻る" };

	// STEP34: 1.5x。「戻る」項目の位置はDraw()参照。STEP37でUpdate()の
	// マウス当たり判定からも参照するのでここに置く。
	const float kCenterX = SCREEN_WIDTH * 0.5f;
	const float kBackY = 960.0f;
	const float kRowH = 69.0f; // 他メニューのkRowHと合わせた、当たり判定用の行の高さ
	const float kHitHalfWidth = 260.0f;
}

void HowToPlayMenu::Update()
{
	MenuSound* menuSound = Manager::GetGameObject<MenuSound>(); // STEP27

	// STEP37: マウスでのホバー選択/クリック決定。
	bool mouseConfirm = false;
	{
		int mx = Input::GetMouseX();
		int my = Input::GetMouseY();
		float top = kBackY - 6.0f;
		float bottom = top + kRowH;
		if (mx >= (kCenterX - kHitHalfWidth) && mx <= (kCenterX + kHitHalfWidth) && my >= top && my <= bottom)
		{
			if (m_Selected != 0)
			{
				m_Selected = 0;
				if (menuSound) menuSound->PlayMove();
			}
			if (Input::GetMouseLeftTrigger())
			{
				mouseConfirm = true;
			}
		}
	}

	if (Input::GetKeyTrigger('W') || Input::GetKeyTrigger(VK_UP))
	{
		m_Selected = (m_Selected + kItemCount - 1) % kItemCount;
		if (menuSound) menuSound->PlayMove();
	}
	else if (Input::GetKeyTrigger('S') || Input::GetKeyTrigger(VK_DOWN))
	{
		m_Selected = (m_Selected + 1) % kItemCount;
		if (menuSound) menuSound->PlayMove();
	}

	// STEP26: このシーンにはPauseMenu/SettingsScreenのようなオーバーレイが
	// 無いため、ESCも「戻る」の即決定として扱ってよい(resultMenu.cppと
	// 同じ考え方)。
	if (Input::GetKeyTrigger(VK_RETURN) || Input::GetKeyTrigger(VK_ESCAPE) || mouseConfirm) // STEP37: マウス決定も同じ扱い
	{
		if (menuSound) menuSound->PlayConfirm(); // STEP27

		if (m_Selected == 0)
		{
			Manager::ChangeScene<Title>();
		}
	}
}

void HowToPlayMenu::Draw()
{
	Hud::Begin();

	Hud::DrawText("操作説明", kCenterX, 90.0f, 51.0f, true); // STEP34: 1.5x

	// STEP31: 追加仕様書「ゲームの説明画面」を、この操作説明画面に
	// 統合する形で追加(タイトルのボタン数は増やさない)。
	Hud::DrawText("目的: 鍵をすべて集めて、廃墟から脱出せよ", kCenterX, 157.5f, 33.0f, true); // STEP34: 1.5x

	// STEP26: 各アイコン(howToPlay.cppでm_Layer=9のPolygon2Dとして配置
	// 済み)の下にラベルを添える。座標はhowToPlay.cpp側のアイコン配置と
	// 対応させてあるので、アイコンの位置を変えるときはこちらのyも合わせる。
	Hud::DrawText("WASD: 移動", 420.0f, 450.0f, 33.0f, true); // STEP34: 1.5x
	Hud::DrawText("マウス: 視点移動", 1125.0f, 465.0f, 33.0f, true); // STEP34: 1.5x
	Hud::DrawText("E: 調べる / 開ける", 420.0f, 810.0f, 33.0f, true); // STEP34: 1.5x
	Hud::DrawText("ESC: 一時停止", 1125.0f, 810.0f, 33.0f, true); // STEP34: 1.5x

	char buf[32];
	sprintf_s(buf, "%s%s", (m_Selected == 0) ? "> " : "  ", kLabels[0]);
	Hud::DrawText(buf, kCenterX, kBackY, 36.0f, true); // STEP34: 1.5x

	// STEP37: シーン遷移の黒フェード(最前面)。
	Hud::DrawFullScreenTint(0.0f, 0.0f, 0.0f, Manager::GetFadeAlpha());

	Hud::End();
}
