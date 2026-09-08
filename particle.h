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

	// STEP13: ambient floating-dust mode -- gentle, gravity-free drift
	// within a box around this object's position, particles quietly
	// recycled instead of a one-shot gravity burst. Off by default, so
	// Explosion.cpp's existing spark-burst behavior is untouched.
	bool m_AmbientMode = false;
	float m_SpawnRadius = 2.0f;
	float m_SpawnHeight = 2.0f;
	int m_AmbientCount = 30; // <= MAX_PARTICLES; kept low so N rooms' worth of these stays cheap to draw

public:

	void Init()override;
	void Uninit()override;
	void Update()override;
	void Draw()override;

	// radius/height define a box (2*radius wide/deep, height tall) around
	// this object's position that motes spawn and drift within. count caps
	// how many of MAX_PARTICLES this instance actually uses (draw cost
	// scales with it) -- call once, right after AddGameObject<Particle>().
	void SetAmbientMode(float radius, float height, int count = 30);
};