#pragma once
#include "gameObject.h"
#include <vector>
#include <functional>

// STEP01(stage2): the underground stage's own loading sequence, mirroring
// loadingScreen.h/cpp exactly (same step-by-step spawn + progress bar UI)
// but spawning UndergroundMap instead of Map, and without Score (stage 1's
// tally doesn't apply here -- STEP07/08's item/generator tracking will
// likely want its own UI later instead of reusing this one).
class UndergroundLoadingScreen : public GameObject
{
private:
	struct Step
	{
		std::function<void()> Action;
		const char* Label;
	};

	std::vector<Step> m_Steps;
	size_t m_StepIndex = 0;

public:
	void Init() override;
	void Update() override;
	void Draw() override;
};
