#pragma once
#include "camera.h"

class MenuCamera : public Camera
{
private:
	float m_TimeAccum = 0.0f;
public:
	void Init() override;
	void Update() override;
};
