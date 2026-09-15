#include "main.h"
#include "titleMenu.h"
#include "settingsScreen.h"
#include "manager.h"
#include "Input.h"
#include "hud.h"
#include "Game.h"
#include <cstdio>

namespace
{
	const int kItemCount = 2;
	const char* kLabels[kItemCount] = { "ゲームスタート", "設定" };
}

void TitleMenu::Update()
{
	SettingsScreen* settings = Manager::GetGameObject<SettingsScreen>();

	// 設定画面が開いている間はそちらが入力を持つ(pauseMenu.cppと同じ考え方)。
	if (settings && settings->IsOpen())
	{
		return;
	}

	if (Input::GetKeyTrigger('W') || Input::GetKeyTrigger(VK_UP))
	{
		m_Selected = (m_Selected + kItemCount - 1) % kItemCount;
	}
	else if (Input::GetKeyTrigger('S') || Input::GetKeyTrigger(VK_DOWN))
	{
		m_Selected = (m_Selected + 1) % kItemCount;
	}

	if (Input::GetKeyTrigger(VK_RETURN))
	{
		if (m_Selected == 0)
		{
			Manager::ChangeScene<Game>();
		}
		else if (m_Selected == 1)
		{
			if (settings) settings->Open();
		}
	}
}

void TitleMenu::Draw()
{
	SettingsScreen* settings = Manager::GetGameObject<SettingsScreen>();

	// Titleシーンには他にHud::Begin()/End()を使うオブジェクトが無いので、
	// interact.cppのような外部からの間接呼び出しは不要 -- ここで直接開閉する。
	Hud::Begin();

	const float centerX = SCREEN_WIDTH * 0.5f;
	float y = SCREEN_HEIGHT * 0.68f;

	for (int i = 0; i < kItemCount; i++)
	{
		char buf[32];
		sprintf_s(buf, "%s%s", (i == m_Selected) ? "> " : "  ", kLabels[i]);
		Hud::DrawText(buf, centerX, y, 26.0f, true);
		y += 46.0f;
	}

	if (settings) settings->DrawUI();

	Hud::End();
}
