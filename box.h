#pragma once

#include "gameObject.h"

class Box : public GameObject
{
private:
	Vector3 m_Velocity{ 0,0,0 };

	ID3D11InputLayout* m_VertexLayout;
	ID3D11VertexShader* m_VertexShader;
	ID3D11PixelShader* m_PixelShader;

public:
	void Init()override;
	void Uninit()override;
	void Update()override;
	void Draw()override;

	// Player.cpp's box-collision loop (Manager::GetGameObjects<Box>()) calls
	// this before resolving a wall's overlap; a subclass that shouldn't
	// always block (Door, while open) overrides it. Defaults to true so
	// every plain Box keeps blocking exactly like before.
	virtual bool IsBlocking() const { return true; }
};