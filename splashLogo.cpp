//splashLogo.cpp
#include "main.h"
#include "splashLogo.h"
#include "polygon2d.h"
#include "manager.h"
#include "Input.h"
#include "title.h"

namespace
{
	const float kLogoWidth = 620.0f;
	const float kLogoHeight = 111.0f;
	const float kLogoX = (SCREEN_WIDTH - kLogoWidth) * 0.5f;
	const float kLogoY = (SCREEN_HEIGHT - kLogoHeight) * 0.5f;

	const float kFadeIn = 0.8f;
	const float kHold = 1.2f;
	const float kFadeOut = 0.8f;
	const float kTotal = kFadeIn + kHold + kFadeOut;
}

void SplashLogo::Init()
{
	m_Timer = 0.0f;
	m_Skipped = false;
	m_ChangedScene = false;

	m_Logo = Manager::AddGameObject<Polygon2D>();
	m_Logo->Init(kLogoX, kLogoY, kLogoWidth, kLogoHeight, L"texture\\logo_splash.png");
	m_Logo->SetAlpha(0.0f);
}

void SplashLogo::Update()
{
	if (!m_Logo || m_ChangedScene) return;

	if (!m_Skipped && (Input::GetKeyTrigger(VK_RETURN) || Input::GetKeyTrigger(VK_SPACE) || Input::GetKeyTrigger(VK_ESCAPE) || Input::GetMouseLeftTrigger()))
	{
		if (m_Timer < kFadeIn + kHold)
		{
			m_Timer = kFadeIn + kHold;
		}
		m_Skipped = true;
	}

	m_Timer += 1.0f / 60.0f;

	float alpha;
	if (m_Timer < kFadeIn)
	{
		alpha = m_Timer / kFadeIn;
	}
	else if (m_Timer < kFadeIn + kHold)
	{
		alpha = 1.0f;
	}
	else if (m_Timer < kTotal)
	{
		float t = (m_Timer - kFadeIn - kHold) / kFadeOut;
		alpha = 1.0f - t;
	}
	else
	{
		alpha = 0.0f;
	}

	if (alpha < 0.0f) alpha = 0.0f;
	if (alpha > 1.0f) alpha = 1.0f;

	m_Logo->SetAlpha(alpha);

	if (m_Timer >= kTotal)
	{
		m_ChangedScene = true;
		Manager::ChangeScene<Title>();
	}
}
