//title.h
#pragma once
#include "menuBackgroundScene.h"

class result : public MenuBackgroundScene
{
public:
	void Init() override;
	void Uninit() override;
	void Update() override;
	void Draw() override;
};