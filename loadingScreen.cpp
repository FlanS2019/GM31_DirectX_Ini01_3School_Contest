//loadingScreen.cpp
#include "main.h"
#include "loadingScreen.h"
#include "manager.h"
#include "hud.h"
#include "Input.h"
#include "camera.h"
#include "light.h"
#include "field.h"
#include "player.h"
#include "Map.h"
#include "Score.h"
#include "bgmPlayer.h"
#include "interact.h"
#include "pauseMenu.h"
#include "settingsScreen.h"
#include "menuSound.h"
#include "horror.h"
#include "gameStartText.h"
#include <cstdio>

void LoadingScreen::Init()
{
	m_Layer = 10;
	m_StepIndex = 0;

	// STEP40: Game::Init()の元の13行と全く同じ順番・内容。GameStartTextは
	// 「ゲームスタート」フェード演出なので、ロード完了直後に見えるよう
	// 必ず最後のステップにする(Input::SetMouseCaptureEnabled(true)も
	// 同じタイミング -- 元のGame::Init()末尾にあったもの)。
	m_Steps.push_back({ []() { Manager::AddGameObject<Camera>(); }, "カメラ" });
	m_Steps.push_back({ []() { Manager::AddGameObject<Light>(); }, "ライト" });
	m_Steps.push_back({ []() { Manager::AddGameObject<Field>(); }, "地面" });
	m_Steps.push_back({ []() { Manager::AddGameObject<Player>(); }, "プレイヤー" });
	m_Steps.push_back({ []() { Manager::AddGameObject<Map>(); }, "マップ" });
	m_Steps.push_back({ []() { Manager::AddGameObject<Score>(); }, "スコア" });
	m_Steps.push_back({ []() { Manager::AddGameObject<BgmPlayer>(); }, "BGM" });
	m_Steps.push_back({ []() { Manager::AddGameObject<Interact>(); }, "インタラクト" });
	m_Steps.push_back({ []() { Manager::AddGameObject<PauseMenu>(); }, "ポーズメニュー" });
	m_Steps.push_back({ []() { Manager::AddGameObject<SettingsScreen>(); }, "設定画面" });
	m_Steps.push_back({ []() { Manager::AddGameObject<MenuSound>(); }, "メニュー効果音" });
	m_Steps.push_back({ []() { Manager::AddGameObject<Horror>(); }, "ホラー演出" });
	m_Steps.push_back({ []() {
			Manager::AddGameObject<GameStartText>();
			Input::SetMouseCaptureEnabled(true);
		}, "準備完了" });
}

void LoadingScreen::Update()
{
	if (m_StepIndex < m_Steps.size())
	{
		m_Steps[m_StepIndex].Action();
		m_StepIndex++;

		if (m_StepIndex >= m_Steps.size())
		{
			SetDestroy(true);
		}
	}
}

void LoadingScreen::Draw()
{
	if (m_StepIndex >= m_Steps.size()) return;

	Hud::Begin();

	Hud::DrawFullScreenTint(0.0f, 0.0f, 0.0f, 1.0f);

	const float barWidth = 480.0f;
	const float barHeight = 28.0f;
	const float barX = (SCREEN_WIDTH - barWidth) * 0.5f;
	const float barY = SCREEN_HEIGHT * 0.55f;

	Hud::DrawFilledRect(barX, barY, barWidth, barHeight, 0.25f, 0.25f, 0.25f, 1.0f);

	float progress = (float)m_StepIndex / (float)m_Steps.size();
	if (progress > 1.0f) progress = 1.0f;

	Hud::DrawFilledRect(barX, barY, barWidth * progress, barHeight, 0.62f, 0.12f, 0.12f, 1.0f);

	char buf[64];
	sprintf_s(buf, "Loading... %d%%", (int)(progress * 100.0f));
	Hud::DrawText(buf, SCREEN_WIDTH * 0.5f, barY - 40.0f, 26.0f, true);

	const char* label = m_Steps[m_StepIndex].Label;
	Hud::DrawText(label, SCREEN_WIDTH * 0.5f, barY + barHeight + 14.0f, 20.0f, true);

	Hud::End();
}
