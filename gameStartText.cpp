#include "main.h"
#include "gameStartText.h"
#include "hud.h"
#include <cstdlib>

namespace
{
	// フェードイン0.6秒 → 1.0秒表示 → フェードアウト1.0秒、で消える。
	const float kFadeIn = 0.6f;
	const float kHold = 1.0f;
	const float kFadeOut = 1.0f;
	const float kTotal = kFadeIn + kHold + kFadeOut;
}

void GameStartText::Init()
{
	m_Timer = 0.0f;
	m_Layer = 10;
}

void GameStartText::Update()
{
	m_Timer += 1.0f / 60.0f;

	if (m_Timer >= kTotal)
	{
		SetDestroy(true);
	}
}

void GameStartText::Draw()
{
	float alpha;

	if (m_Timer < kFadeIn)
	{
		alpha = m_Timer / kFadeIn;
	}
	else if (m_Timer < kFadeIn + kHold)
	{
		alpha = 1.0f;
	}
	else
	{
		float t = (m_Timer - kFadeIn - kHold) / kFadeOut;
		alpha = 1.0f - t;
	}

	if (alpha < 0.0f) alpha = 0.0f;
	if (alpha > 1.0f) alpha = 1.0f;

	Hud::Begin();

	// STEP36: 「ゲームスタート」だと味気ないとのことで文言変更。
	// ホラー演出として、表示が安定しているはずの区間でもたまに1フレーム
	// だけ描画を飛ばして「不安定にチラつく」感じを出す(rand()で軽く)。
	bool flicker = (rand() % 14 == 0);
	if (!flicker)
	{
		Hud::DrawTextAlpha("廃墟、はじめ", SCREEN_WIDTH * 0.5f, SCREEN_HEIGHT * 0.42f, 72.0f, true, alpha); // STEP34: 1.5x
	}

	Hud::End();
}
