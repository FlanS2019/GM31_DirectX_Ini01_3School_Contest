#pragma once

#include "gameObject.h"
#include "vector3.h"
#include <d3d11.h>

class Bullet : public GameObject
{
private:

	ID3D11VertexShader* m_VertexShader = nullptr;
	ID3D11InputLayout* m_VertexLayout = nullptr;
	ID3D11PixelShader* m_PixelShader = nullptr;

	Vector3 m_Velocity{ 0, 0, 0 };
	float m_Lifetime = 2.0f; // íeÇÃéıñΩÅiïbÅj

public:
	void Init() override;
	void Uninit() override;
	void Update() override;
	void Draw() override;

	void SetVelocity(const Vector3& velocity) { m_Velocity = velocity; }
	void SetLifetime(float lifetime) { m_Lifetime = lifetime; }
};