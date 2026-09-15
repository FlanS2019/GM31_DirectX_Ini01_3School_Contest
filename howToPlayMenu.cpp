#include "main.h"
#include "howToPlayMenu.h"
#include "manager.h"
#include "Input.h"
#include "hud.h"
#include "title.h"
#include "menuSound.h" 
#include <cstdio>

namespace
{
	const int kItemCount = 1;
	const char* kLabels[kItemCount] = { "戻る" };

	const float kCenterX = SCREEN_WIDTH * 0.5f;
	const float kBackY = 960.0f;
	const float kRowH = 69.0f; 
	const float kHitHalfWidth = 260.0f;
}

void HowToPlayMenu::Update()
{
	MenuSound* menuSound = Manager::GetGameObject<MenuSound>();

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

	if (Input::GetKeyTrigger(VK_RETURN) || Input::GetKeyTrigger(VK_ESCAPE) || mouseConfirm)
	{
		if (menuSound) menuSound->PlayConfirm();

		if (m_Selected == 0)
		{
			Manager::ChangeScene<Title>();
		}
	}
}

void HowToPlayMenu::Draw()
{
	Hud::Begin();

	Hud::DrawText("操作説明", kCenterX, 90.0f, 51.0f, true); 

	Hud::DrawText("目的: 鍵をすべて集めて、廃墟から脱出せよ", kCenterX, 157.5f, 33.0f, true);

	Hud::DrawText("WASD: 移動", 420.0f, 450.0f, 33.0f, true); 
	Hud::DrawText("マウス: 視点移動", 1125.0f, 465.0f, 33.0f, true); 
	Hud::DrawText("E: 調べる / 開ける", 420.0f, 810.0f, 33.0f, true); 
	Hud::DrawText("ESC: 一時停止", 1125.0f, 810.0f, 33.0f, true);

	char buf[32];
	sprintf_s(buf, "%s%s", (m_Selected == 0) ? "> " : "  ", kLabels[0]);
	Hud::DrawText(buf, kCenterX, kBackY, 36.0f, true); 

	Hud::DrawFullScreenTint(0.0f, 0.0f, 0.0f, Manager::GetFadeAlpha());

	Hud::End();
}
