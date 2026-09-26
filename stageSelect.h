#pragma once
#include "menuBackgroundScene.h"

// STEP(stage-select): the stage-picker screen, reached from TitleMenu's new
// "ステージ選択" row. Built exactly like Title/result -- a
// MenuBackgroundScene subclass plus one UI GameObject (StageSelectMenu) --
// so it gets the same live 3D backdrop + grey filter "for free".
class StageSelect : public MenuBackgroundScene
{
public:
	void Init() override;
	void Uninit() override;
	void Update() override {}
	void Draw() override {}
};
