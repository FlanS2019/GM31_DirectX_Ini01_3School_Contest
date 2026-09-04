#pragma once
#include "gameObject.h"

class Camera : public GameObject
{
private:
	Vector3 m_Target{ 0, 0, 0 };

	// First-person look angles (radians)
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