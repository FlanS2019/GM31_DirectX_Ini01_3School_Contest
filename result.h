//title.h
#pragma once
#include "Scene.h"

class result : public Scene
{
public:
	void Init() override;
	void Uninit() override;
	void Update() override;
	void Draw() override;
};