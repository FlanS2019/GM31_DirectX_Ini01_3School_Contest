//title.h
#pragma once
#include "menuBackgroundScene.h"

class Title : public MenuBackgroundScene
{
public:
	void Init() override;
	void Uninit() override;
	void Update() override;
	void Draw() override;
};