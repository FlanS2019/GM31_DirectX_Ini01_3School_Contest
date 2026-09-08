#pragma once

#include "gameObject.h"

class Box : public GameObject
{
private:
	Vector3 m_Velocity{ 0,0,0 };

	ID3D11InputLayout* m_VertexLayout;
	ID3D11VertexShader* m_VertexShader;
	ID3D11PixelShader* m_PixelShader;

	bool m_Blocking = true;

public:
	void Init()override;
	void Uninit()override;
	void Update()override;
	void Draw()override;

	virtual bool IsBlocking() const { return m_Blocking; }

	void SetBlocking(bool blocking) { m_Blocking = blocking; }
};
