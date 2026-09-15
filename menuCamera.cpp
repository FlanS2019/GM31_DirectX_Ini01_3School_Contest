#include "main.h"
#include "menuCamera.h"

namespace
{
	const float kEyeHeight = 1.6f;
	const float kSwayPeriod = 14.0f;    // ñU‚è1‰•œ‚É‚©‚©‚é•b”
	const float kSwayAmplitude = 0.25f; // ñU‚è‚ÌU‚ê•(ƒ‰ƒWƒAƒ“)
	const float kBasePitch = -0.05f;    // ‹C‚¿‰ºŒü‚«

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

	m_Position = Vector3(-2.0f, kEyeHeight, -4.0f);
	m_Yaw = 0.0f;
	m_Pitch = kBasePitch;

	m_Target = m_Position + ForwardFromAngles(m_Yaw, m_Pitch);
}

void MenuCamera::Update()
{
	const float dt = 1.0f / 60.0f;
	m_TimeAccum += dt;

	m_Yaw = sinf(m_TimeAccum * (XM_2PI / kSwayPeriod)) * kSwayAmplitude;
	m_Pitch = kBasePitch;

	m_Target = m_Position + ForwardFromAngles(m_Yaw, m_Pitch);
}
