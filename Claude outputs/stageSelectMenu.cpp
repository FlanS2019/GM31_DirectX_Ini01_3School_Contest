//stageSelectMenu.cpp
#include "main.h"
#include "stageSelectMenu.h"
#include "settingsScreen.h"
#include "manager.h"
#include "Input.h"
#include "hud.h"
#include "Game.h"
#include "underground.h"
#include "menuCamera.h"
#include "gameSettings.h"
#include "menuSound.h"
#include "title.h"
#include <cstdio>
#include <cstring>
#include <cmath>

namespace
{
	struct StageInfo
	{
		const char* Name;
		int RequiresStageCleared; // -1 = always unlocked; otherwise an index into this same array
	};

	// STEP(stage-select): add a row here (and bump kStageCount, and the
	// Update() confirm switch below) for every future stage.
	const StageInfo kStages[] =
	{
		{ "廃墟", -1 }, // 廃墟 (stage 0) -- always unlocked
		{ "地下",  0 }, // 地下 (stage 1) -- needs stage 0 cleared
	};
	const int kStageCount = 2;

	const float kCardWidth = 360.0f;
	const float kCardHeightIdle = 220.0f;
	const float kCardHeightHover = 260.0f;
	const float kCardGap = 60.0f;
	const float kCardCenterY = SCREEN_HEIGHT * 0.5f;

	// STEP01-fix(stage2): same fade length the old in-map underground door
	// used to pass into ChangeScene<Underground>() -- see door.cpp's history.
	const float kUndergroundEntryFadeSeconds = 3.0f;

	// STEP(stage-select-horror): overall darkening of the screen edges while
	// this menu is up -- Hud::DrawVignette() is the same primitive horror.cpp
	// would use during play, just dialed in a bit lighter here so the cards
	// stay readable.
	const float kVignetteStrength = 0.55f;

	bool IsUnlocked(int index)
	{
		int req = kStages[index].RequiresStageCleared;
		return req < 0 || GameSettings::IsStageCleared(req);
	}

	void GetCardRect(int index, bool hovered, float& outX, float& outY, float& outW, float& outH)
	{
		float totalWidth = kStageCount * kCardWidth + (kStageCount - 1) * kCardGap;
		float startX = (SCREEN_WIDTH - totalWidth) * 0.5f;

		outW = kCardWidth;
		outH = hovered ? kCardHeightHover : kCardHeightIdle;
		outX = startX + index * (kCardWidth + kCardGap);
		outY = kCardCenterY - outH * 0.5f;
	}

	// Hit-tested against the IDLE size always, even for the currently-hovered
	// card -- testing against the enlarged size would make the hit box grow
	// the instant it's hovered, which feels twitchy right at the edge.
	bool HitTestCard(int mx, int my, int index)
	{
		float x, y, w, h;
		GetCardRect(index, false, x, y, w, h);
		return mx >= x && mx <= x + w && my >= y && my <= y + h;
	}
}

void StageSelectMenu::Init()
{
	m_Layer = 10;
	m_Selected = 0;
	m_WarningTimer = 0.0f;
	m_WarningText[0] = '\0';
	m_FlickerTime = 0.0f;
}

void StageSelectMenu::Update()
{
	SettingsScreen* settings = Manager::GetGameObject<SettingsScreen>();
	MenuSound* menuSound = Manager::GetGameObject<MenuSound>();

	m_FlickerTime += 1.0f / 60.0f;

	if (m_WarningTimer > 0.0f)
	{
		m_WarningTimer -= 1.0f / 60.0f;
		if (m_WarningTimer < 0.0f) m_WarningTimer = 0.0f;
	}

	if (settings && settings->IsOpen())
	{
		return;
	}

	bool mouseConfirm = false;
	bool anyHovered = false;
	{
		int mx = Input::GetMouseX();
		int my = Input::GetMouseY();

		for (int i = 0; i < kStageCount; i++)
		{
			if (HitTestCard(mx, my, i))
			{
				anyHovered = true;
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
		}
	}

	// STEP(stage-select): "マウスカーソルを持ってきたら背景が動く" -- the idle sway
	// intensifies while any card is hovered, and eases back down otherwise.
	// See MenuCamera::SetSwayIntensityTarget()'s comment for the easing.
	{
		MenuCamera* menuCamera = Manager::GetGameObject<MenuCamera>();
		if (menuCamera) menuCamera->SetSwayIntensityTarget(anyHovered ? 3.0f : 1.0f);
	}

	if (Input::GetKeyTrigger('A') || Input::GetKeyTrigger(VK_LEFT))
	{
		m_Selected = (m_Selected + kStageCount - 1) % kStageCount;
		if (menuSound) menuSound->PlayMove();
	}
	else if (Input::GetKeyTrigger('D') || Input::GetKeyTrigger(VK_RIGHT))
	{
		m_Selected = (m_Selected + 1) % kStageCount;
		if (menuSound) menuSound->PlayMove();
	}

	if (Input::GetKeyTrigger(VK_ESCAPE))
	{
		Manager::ChangeScene<Title>();
		return;
	}

	if (Input::GetKeyTrigger(VK_RETURN) || mouseConfirm)
	{
		if (!IsUnlocked(m_Selected))
		{
			strcpy_s(m_WarningText, "先に前のステージをクリアしてください");
			m_WarningTimer = 2.0f;
			return;
		}

		if (menuSound) menuSound->PlayConfirm();

		// STEP(stage-select): add a case here for every future stage (kept as
		// an explicit switch, not an array of scene-changers, since each
		// stage's scene type is different -- Game vs Underground vs whatever
		// comes next).
		switch (m_Selected)
		{
		case 0: // 廃墟
			Manager::ChangeScene<Game>();
			break;
		case 1: // 地下
			// STEP01-fix(stage2): same transition beat the old map door used to
			// trigger -- see Underground::s_TransitionActive's comment.
			Underground::SetTransitionActive(true);
			Manager::ChangeScene<Underground>(kUndergroundEntryFadeSeconds);
			break;
		}
	}
}

void StageSelectMenu::Draw()
{
	SettingsScreen* settings = Manager::GetGameObject<SettingsScreen>();

	Hud::Begin();

	// STEP(stage-select-horror): darken the edges before drawing anything
	// else, so the cards/text below sit on top of it like the rest of the
	// game's UI does over horror.cpp's vignette during play.
	Hud::DrawVignette(kVignetteStrength);

	// Irregular flicker multiplier for the hovered/unlocked card -- product
	// of two out-of-sync sine waves instead of one, so it reads as an
	// unstable light rather than a metronome pulse.
	float flicker = 0.82f + 0.18f * sinf(m_FlickerTime * 9.0f) * sinf(m_FlickerTime * 2.3f + 1.0f);

	for (int i = 0; i < kStageCount; i++)
	{
		bool hovered = (i == m_Selected);
		bool unlocked = IsUnlocked(i);

		float x, y, w, h;
		GetCardRect(i, hovered, x, y, w, h);

		// STEP(stage-select-horror): a soft blood-red glow just outside the
		// card, only while it's hovered and unlocked -- reads as a dying
		// light source rather than a UI highlight.
		if (hovered && unlocked)
		{
			float glowPad = 16.0f;
			Hud::DrawFilledRect(x - glowPad, y - glowPad, w + glowPad * 2.0f, h + glowPad * 2.0f,
				0.35f, 0.02f, 0.02f, 0.16f * flicker);
		}

		// STEP(stage-select-horror): "もっと怖く" -- swapped the cheerful
		// grey/orange palette for cold near-black (idle), a flickering
		// blood red (hovered+unlocked), and a dead, motionless near-black
		// for locked cards (deliberately does NOT flicker -- the stillness
		// next to the flickering unlocked card is the point).
		float r, g, b, a;
		if (!unlocked) { r = 0.05f; g = 0.04f; b = 0.045f; a = 0.82f; }
		else if (hovered) { r = 0.55f * flicker; g = 0.04f * flicker; b = 0.04f * flicker; a = 0.88f; }
		else { r = 0.12f; g = 0.10f; b = 0.11f; a = 0.75f; }

		Hud::DrawFilledRect(x, y, w, h, r, g, b, a);

		char buf[64];
		sprintf_s(buf, "%s%s", (hovered && unlocked) ? "> " : "  ", kStages[i].Name);
		Hud::DrawText(buf, x + w * 0.5f, y + h * 0.5f - 20.0f, hovered ? 40.0f : 32.0f, true);

		if (!unlocked)
		{
			Hud::DrawText("ロック中", x + w * 0.5f, y + h * 0.5f + 30.0f, 22.0f, true);
		}
	}

	if (m_WarningTimer > 0.0f)
	{
		Hud::DrawText(m_WarningText, SCREEN_WIDTH * 0.5f, kCardCenterY + kCardHeightHover * 0.5f + 50.0f, 28.0f, true);
	}

	// STEP(stage-select-back-hint): "戻れるけどわかりにくい" -- a persistent,
	// always-visible label in the corner. Hud::DrawText() draws its own
	// background panel, so this stays legible over the dark backdrop/
	// vignette without needing a custom box.
	Hud::DrawText("[ESC] タイトルへ戻る", 30.0f, SCREEN_HEIGHT - 56.0f, 22.0f, false);

	if (settings) settings->DrawUI();

	Hud::DrawFullScreenTint(0.0f, 0.0f, 0.0f, Manager::GetFadeAlpha());

	Hud::End();
}
