#pragma once

#include "menuBackgroundScene.h"

class HowToPlay : public MenuBackgroundScene
{
public:
	void Init() override;
	void Uninit() override;
	void Update() override;
	void Draw() override;
};
