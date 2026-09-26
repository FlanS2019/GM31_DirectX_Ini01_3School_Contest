//undergroundLoadingScreen.cpp
#include "main.h"
#include "undergroundLoadingScreen.h"
#include "manager.h"
#include "hud.h"
#include "Input.h"
#include "camera.h"
#include "light.h"
#include "undergroundFloor.h"
#include "player.h"
#include "undergroundMap.h"
#include "bgmPlayer.h"
#include "interact.h"
#include "pauseMenu.h"
#include "settingsScreen.h"
#include "menuSound.h"
#include "horror.h"
#include "gameStartText.h"
#include <cstdio>

void UndergroundLoadingScreen::Init()
{
	m_Layer = 10;
	m_StepIndex = 0;

	m_Steps.push_back({ []() { Manager::AddGameObject<Camera>(); }, "カメラ" });
	m_Steps.push_back({ []() {
			Light* light = Manager::AddGameObject<Light>();

			// STEP03(stage2): "暗所・照明・明暗差" -- a darker, colder
			// ambient/diffuse than stage 1's defaults. This only touches
			// this scene's own Light instance; Game's Light (spawned fresh
			// by loadingScreen.cpp) is completely separate and unaffected.
			light->SetAmbient(XMFLOAT4(0.010f, 0.010f, 0.013f, 1.0f));
			light->SetDiffuse(XMFLOAT4(0.045f, 0.045f, 0.05f, 1.0f));
		}, "ライト" });
	// STEP02(stage2)-fix: UndergroundFloor (texture\UDG.jpg, sized to cover
	// the whole underground map) instead of Field -- see undergroundFloor.h/cpp.
	m_Steps.push_back({ []() { Manager::AddGameObject<UndergroundFloor>(); }, "地面" });
	m_Steps.push_back({ []() { Manager::AddGameObject<Player>(); }, "プレイヤー" });
	m_Steps.push_back({ []() { Manager::AddGameObject<UndergroundMap>(); }, "地下マップ" });
	m_Steps.push_back({ []() { Manager::AddGameObject<BgmPlayer>(); }, "BGM" });
	m_Steps.push_back({ []() { Manager::AddGameObject<Interact>(); }, "インタラクト" });
	m_Steps.push_back({ []() { Manager::AddGameObject<PauseMenu>(); }, "ポーズメニュー" });
	m_Steps.push_back({ []() { Manager::AddGameObject<SettingsScreen>(); }, "設定画面" });
	m_Steps.push_back({ []() { Manager::AddGameObject<MenuSound>(); }, "メニュー効果音" });
	m_Steps.push_back({ []() { Manager::AddGameObject<Horror>(); }, "ホラー演出" });
	m_Steps.push_back({ []() {
			// STEP02(stage2)-fix: "地下ステージスタートのとき、廃墟、はじめ
			// ではなく地下、はじめに変更で" -- GameStartText used to always
			// show stage 1's own hardcoded text no matter which stage
			// spawned it (loadingScreen.cpp and this file both add one at
			// the end of their step lists). See GameStartText::m_Label's
			// comment.
			GameStartText* text = Manager::AddGameObject<GameStartText>();
			text->SetLabel("地下、はじめ");
			Input::SetMouseCaptureEnabled(true);
		}, "準備完了" });
}

void UndergroundLoadingScreen::Update()
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

void UndergroundLoadingScreen::Draw()
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

