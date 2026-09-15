#pragma once
#include <d3d11.h>
#include "vector3.h"
#include "gameObject.h"

class Particle : public GameObject
{
private:

	ID3D11Buffer* m_VertexBuffer;

	ID3D11InputLayout* m_VertexLayout;
	ID3D11VertexShader* m_VertexShader;
	ID3D11PixelShader* m_PixelShader;

	ID3D11ShaderResourceView* m_Texture;

	struct PARTICLE
	{
		bool Enable;
		int Life;
		Vector3 Position;
		Vector3 Velocity;
	};

	static const int MAX_PARTICLES = 100;
	PARTICLE m_Particle[MAX_PARTICLES];

	bool m_AmbientMode = false;
	float m_SpawnRadius = 2.0f;
	float m_SpawnHeight = 2.0f;
	int m_AmbientCount = 30;

public:

	void Init()override;
	void Uninit()override;
	void Update()override;
	void Draw()override;

	void SetAmbientMode(float radius, float height, int count = 30);
};