#pragma once
#include "gameObject.h"

class GameStartText : public GameObject
{
private:
	float m_Timer = 0.0f;
public:
	void Init() override;
	void Update() override;
	void Draw() override;
};
