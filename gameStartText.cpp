#include "main.h"
#include "gameStartText.h"
#include "hud.h"
#include <cstdlib>

namespace
{
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

	bool flicker = (rand() % 14 == 0);
	if (!flicker)
	{
		Hud::DrawTextAlpha("îpö–ÅAÇÕÇ∂Çﬂ", SCREEN_WIDTH * 0.5f, SCREEN_HEIGHT * 0.42f, 72.0f, true, alpha); 
	}

	Hud::End();
}
