#pragma once
#include <d3d11.h>
#include "gameObject.h"

class Score : public GameObject
{
private:

	ID3D11Buffer* m_VertexBuffer;

	ID3D11InputLayout* m_VertexLayout;
	ID3D11VertexShader* m_VertexShader;
	ID3D11PixelShader* m_PixelShader;

	ID3D11ShaderResourceView* m_Texture;

	int m_Value;

public:
	void Init()override;
	void Uninit()override;
	void Update()override;
	void Draw()override;

	void AddScore(int add) { m_Value += add; }
	int  GetValue() const { return m_Value; }

};
