#pragma once
#include "camera.h"

class MenuCamera : public Camera
{
private:
	float m_TimeAccum = 0.0f;

	// STEP(stage-select): current vs. target multiplier on the idle sway
	// below -- eased toward the target each frame in Update() instead of
	// snapping, so a hovered stage card makes the background visibly react
	// ("”wŒi‚ª“®‚­") rather than jump-cutting. 1.0 = the original constant
	// idle sway; nothing outside SetSwayIntensityTarget() ever changes the
	// target, so Title/result behave exactly as before.
	float m_SwayIntensity = 1.0f;
	float m_SwayIntensityTarget = 1.0f;
public:
	void Init() override;
	void Update() override;

	void SetSwayIntensityTarget(float target) { m_SwayIntensityTarget = target; }
};
