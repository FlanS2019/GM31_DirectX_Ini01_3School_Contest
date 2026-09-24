#pragma once
#include "Scene.h"

// STEP01(stage2): the underground exploration stage's own Scene, the direct
// counterpart to Game.h/cpp for the first stage. Kept completely separate
// from Game so nothing about the first stage has to change -- entering here
// is just another Manager::ChangeScene<Underground>() call, the exact same
// mechanism Door already uses to reach `result`.
class Underground : public Scene
{
private:
	// STEP01-fix(stage2): true for the moment the game is transitioning
	// into Underground (fade-out after Door::Open() finishes, or the debug
	// F4 respawn) -- Interact::Draw() reads this to show a "descending the
	// stairs..." line over the fade tint. Cleared back to false the instant
	// Init() actually runs, since by then the loading screen's own black
	// overlay has taken over.
	static bool s_TransitionActive;

public:
	void Init() override;
	void Uninit() override;
	void Update() override;
	void Draw() override;

	static void SetTransitionActive(bool active) { s_TransitionActive = active; }
	static bool IsTransitionActive() { return s_TransitionActive; }
};
