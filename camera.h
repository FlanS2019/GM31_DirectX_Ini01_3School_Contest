#pragma once
#include "gameObject.h"

class Camera : public GameObject
{
private:
	Vector3 m_Target{ 0, 0, 0 };
	//// ’Ç‰Á
	//float m_Yaw = 0.0f;
	//float m_Pitch = 0.3f;
	//float m_Distance = 8.0f;
	XMMATRIX m_ViewMatrix;

public:
	void Init()override;
	void Uninit()override;
	void Update()override;
	void Draw()override;
	Vector3 GetTarget() const { return m_Target; }
    //float GetYaw() const { return m_Yaw; }
	XMMATRIX GetViewMatrix() {
		return m_ViewMatrix;
	}
	Vector3 GetForward()
	{
		// XMVECTOR ‚©‚ç Vector3 ‚Ö•ÏŠ·
		Vector3 forward = m_Target - m_Position;
		forward.normalize();
		return forward;
	}
};

