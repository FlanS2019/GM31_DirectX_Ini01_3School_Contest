#pragma once
#include "gameObject.h"

// STEP(stage-select): the actual stage-card UI, modeled closely on
// TitleMenu (same mouse-hover-drives-m_Selected pattern, same
// Hud::Begin()/End() draw shape) -- see stageSelectMenu.cpp.
class StageSelectMenu : public GameObject
{
private:
	int m_Selected = 0;

	float m_WarningTimer = 0.0f;
	char m_WarningText[128] = {};

public:
	void Init() override;
	void Uninit() override {}
	void Update() override;
	void Draw() override;
};
