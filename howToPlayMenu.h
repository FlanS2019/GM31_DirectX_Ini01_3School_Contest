#pragma once

#include "gameObject.h"

class HowToPlayMenu : public GameObject
{
private:
	int m_Selected = 0;

public:
	void Init() override { m_Selected = 0; m_Layer = 10; }
	void Uninit() override {}
	void Update() override;
	void Draw() override;
};
