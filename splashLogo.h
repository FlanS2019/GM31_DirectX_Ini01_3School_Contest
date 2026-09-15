#pragma once
#include "gameObject.h"

class Polygon2D;

class SplashLogo : public GameObject
{
private:
	Polygon2D* m_Logo = nullptr;
	float m_Timer = 0.0f;
	bool m_Skipped = false;
	bool m_ChangedScene = false;
public:
	void Init() override;
	void Update() override;
};
