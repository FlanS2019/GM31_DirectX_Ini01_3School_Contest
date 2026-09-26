#include "main.h"
#include "menuCamera.h"

namespace
{
	const float kEyeHeight = 1.6f;
	const float kSwayPeriod = 14.0f;    // ŽñU‚è1‰•œ‚É‚©‚©‚é•b”
	const float kSwayAmplitude = 0.25f; // ŽñU‚è‚ÌU‚ê•(ƒ‰ƒWƒAƒ“)
	const float kBasePitch = -0.05f;    // ‹CŽ‚¿‰ºŒü‚«

	Vector3 ForwardFromAngles(float yaw, float pitch)
	{
		Vector3 forward;
		forward.x = cosf(pitch) * sinf(yaw);
		forward.y = sinf(pitch);
		forward.z = cosf(pitch) * cosf(yaw);
		forward.normalize();
		return forward;
	}
}

void MenuCamera::Init()
{
	m_TimeAccum = 0.0f;

	// STEP(stage-select): always start a fresh Title/result/StageSelect at
	// the normal idle sway -- never carry over a boosted value from
	// whichever menu scene was showing before this one.
	m_SwayIntensity = 1.0f;
	m_SwayIntensityTarget = 1.0f;

	m_Position = Vector3(-2.0f, kEyeHeight, -4.0f);
	m_Yaw = 0.0f;
	m_Pitch = kBasePitch;

	m_Target = m_Position + ForwardFromAngles(m_Yaw, m_Pitch);
}

void MenuCamera::Update()
{
	const float dt = 1.0f / 60.0f;
	m_TimeAccum += dt;

	// STEP(stage-select): ease m_SwayIntensity toward its target instead of
	// snapping -- see the fields' comment in menuCamera.h.
	{
		const float kEaseSpeed = 2.0f; // per second
		float diff = m_SwayIntensityTarget - m_SwayIntensity;
		float ease = kEaseSpeed * dt;
		if (ease > 1.0f) ease = 1.0f;
		m_SwayIntensity += diff * ease;
	}

	m_Yaw = sinf(m_TimeAccum * (XM_2PI / kSwayPeriod) * m_SwayIntensity) * kSwayAmplitude * m_SwayIntensity;
	m_Pitch = kBasePitch;

	m_Target = m_Position + ForwardFromAngles(m_Yaw, m_Pitch);
}
