#include "main.h"
#include "resultMenu.h"
#include "manager.h"
#include "Input.h"
#include "hud.h"
#include "gameStats.h"
#include "title.h"
#include "menuSound.h" 
#include <cstdio>

namespace
{
	const int kItemCount = 1;
	const char* kLabels[kItemCount] = { "タイトルへ戻る" };

	const float kCenterX = SCREEN_WIDTH * 0.5f;
	const float kItemRowH = 60.0f; 
	const float kHitHalfWidth = 260.0f;
}

void ResultMenu::Update()
{
	MenuSound* menuSound = Manager::GetGameObject<MenuSound>(); 

	bool mouseConfirm = false;
	{
		int mx = Input::GetMouseX();
		int my = Input::GetMouseY();

		float y = SCREEN_HEIGHT * 0.35f;
		y += 90.0f; // "脱出成功"見出しの後
		y += 54.0f; // 生還までの時間の後
		y += 90.0f; // 見つけた鍵の後(ここが最初の項目のY)

		for (int i = 0; i < kItemCount; i++)
		{
			float top = y - 6.0f;
			float bottom = top + kItemRowH;
			if (mx >= (kCenterX - kHitHalfWidth) && mx <= (kCenterX + kHitHalfWidth) && my >= top && my <= bottom)
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
			y += kItemRowH;
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

void ResultMenu::Draw()
{
	Hud::Begin();

	const float centerX = SCREEN_WIDTH * 0.5f;
	float y = SCREEN_HEIGHT * 0.35f;

	Hud::DrawText("脱出成功", centerX, y, 54.0f, true); 
	y += 90.0f;

	int elapsedTotalSeconds = (int)GameStats::GetElapsedSeconds();
	int minutes = elapsedTotalSeconds / 60;
	int seconds = elapsedTotalSeconds % 60;

	char timeBuf[64];
	sprintf_s(timeBuf, "生還までの時間: %d分%02d秒", minutes, seconds);
	Hud::DrawText(timeBuf, centerX, y, 33.0f, true); 
	y += 54.0f; 

	char keyBuf[64];
	sprintf_s(keyBuf, "見つけた鍵: %d / %d個", GameStats::GetKeysCollected(), GameStats::GetTotalKeys());
	Hud::DrawText(keyBuf, centerX, y, 33.0f, true); 
	y += 90.0f;

	for (int i = 0; i < kItemCount; i++)
	{
		char buf[32];
		sprintf_s(buf, "%s%s", (i == m_Selected) ? "> " : "  ", kLabels[i]);
		Hud::DrawText(buf, centerX, y, 36.0f, true);
		y += 60.0f;
	}

	Hud::DrawFullScreenTint(0.0f, 0.0f, 0.0f, Manager::GetFadeAlpha());

	Hud::End();
}
