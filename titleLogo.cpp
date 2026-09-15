//titleLogo.cpp
#include "main.h"
#include "titleLogo.h"
#include "polygon2d.h"
#include "manager.h"

namespace
{
	// logo_title.png は900x380(比率2.368:1)。画面(1280x720)上部中央に、
	// タイトルメニュー(titleMenu.cpp kRootStartY = SCREEN_HEIGHT*0.55 = 396)
	// の上に十分な余白を残して置く。
	const float kLogoWidth = 580.0f;
	const float kLogoHeight = 245.0f; // 900:380の比率を保ったまま縮小
	const float kLogoX = (SCREEN_WIDTH - kLogoWidth) * 0.5f;
	const float kLogoY = 45.0f;

	const float kFadeInDuration = 1.2f;
}

void TitleLogo::Init()
{
	m_Timer = 0.0f;
	m_Logo = Manager::AddGameObject<Polygon2D>();
	m_Logo->Init(kLogoX, kLogoY, kLogoWidth, kLogoHeight, L"texture\\logo_title.png");
	m_Logo->SetAlpha(0.0f);
}

void TitleLogo::Update()
{
	if (!m_Logo) return;

	m_Timer += 1.0f / 60.0f;

	float alpha = m_Timer / kFadeInDuration;
	if (alpha > 1.0f) alpha = 1.0f;

	m_Logo->SetAlpha(alpha);
}
