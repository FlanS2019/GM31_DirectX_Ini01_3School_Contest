#pragma once
#include "gameObject.h"

class Polygon2D;

class TitleLogo : public GameObject
{
private:
	Polygon2D* m_Logo = nullptr;
	float m_Timer = 0.0f;
public:
	void Init() override;
	void Update() override;
};
