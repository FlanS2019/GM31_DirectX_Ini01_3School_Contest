#pragma once
#include "gameObject.h"

class Camera : public GameObject
{
public:
	// STEP51: named versions of the constants Draw() passes to
	// XMMatrixPerspectiveFovLH() -- pulled out so the culling code in
	// manager.cpp can derive its view cone from the SAME numbers instead of
	// duplicating them (and silently drifting out of sync if the FOV or far
	// clip is ever retuned).
	static constexpr float kFovY = 1.0f;     // vertical field of view, radians
	static constexpr float kNearClip = 0.1f;
	static constexpr float kFarClip = 70.0f;

protected: 
	Vector3 m_Target{ 0, 0, 0 };

	float m_Yaw = 0.0f;   // left/right
	float m_Pitch = 0.0f; // up/down

	XMMATRIX m_ViewMatrix;
	XMMATRIX m_ProjectionMatrix;
public:
	void Init()override;
	void Uninit()override;
	void Update()override;
	void Draw()override;

	Vector3 GetTarget() const { return m_Target; }

	float GetYaw() const { return m_Yaw; }
	float GetPitch() const { return m_Pitch; }

	XMMATRIX GetViewMatrix() {
		return m_ViewMatrix;
	}
	XMMATRIX GetProjectionMatrix() {
		return m_ProjectionMatrix;
	}
	Vector3 GetForward()
	{
		Vector3 forward = m_Target - m_Position;
		forward.normalize();
		return forward;
	}
};