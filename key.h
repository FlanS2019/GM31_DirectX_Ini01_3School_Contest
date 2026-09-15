#pragma once

#include "gameObject.h"

class Key : public GameObject
{
private:
	int m_KeyId = 0;

	ID3D11InputLayout* m_VertexLayout = nullptr;
	ID3D11VertexShader* m_VertexShader = nullptr;
	ID3D11PixelShader* m_PixelShader = nullptr;

public:
	void Init()override;
	void Uninit()override;
	void Update()override;
	void Draw()override;

	void SetKeyId(int id) { m_KeyId = id; }
};
