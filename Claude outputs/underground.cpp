//underground.cpp
#include "main.h"
#include "manager.h"
#include "underground.h"
#include "undergroundLoadingScreen.h"
#include "player.h"
#include "undergroundMap.h"
#include "Input.h"

bool Underground::s_TransitionActive = false; // STEP01-fix(stage2)

void Underground::Init()
{
	// STEP01-fix(stage2): the transition beat (if any) is over now that the
	// real scene is starting -- see the flag's comment in underground.h.
	s_TransitionActive = false;

	// STEP01(stage2): the underground's own respawn point -- where the player
	// lands both on first entry and (from STEP09-11 onward, once the real
	// failure system exists) after a failure sends them back here.
	// UndergroundMap::GetSpawnPosition() is the single source of truth for
	// this position so the two can never drift apart -- see its comment.
	Player::SetSpawnPosition(UndergroundMap::GetSpawnPosition());

	Manager::AddGameObject<UndergroundLoadingScreen>();
}

void Underground::Uninit()
{
	Input::SetMouseCaptureEnabled(false);
}

void Underground::Update()
{
	// STEP01(stage2) TEMPORARY: there's no real failure system yet (that's
	// STEP09 "失敗判定" through STEP11 "リスポーン"), but STEP01's own completion
	// condition is "地下へ移動でき、失敗時に戻れる" (can reach the underground, and can
	// return on failure) -- this lets that second half be tested right now.
	// F4 re-enters this same scene exactly like a real failure->respawn will:
	// Manager::ChangeScene<Underground>() resets every GameObject and re-runs
	// Init() above, landing the player back on the spawn point.
	// DELETE this block once STEP09's real failure trigger exists.
	if (Input::GetKeyTrigger(VK_F4))
	{
		SetTransitionActive(true); // STEP01-fix(stage2): same beat as the real door, for a consistent test
		Manager::ChangeScene<Underground>(0.5f);
	}
}

void Underground::Draw()
{
}
