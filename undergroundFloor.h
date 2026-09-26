#pragma once
#include <d3d11.h>
#include "vector3.h"
#include "gameObject.h"

// STEP02(stage2)-fix: the underground's own ground plane -- a straight copy
// of Field.h/cpp's shape (one textured quad, same shaders), just sized to
// cover the whole underground area (entrance room + STEP02's grid) instead
// of Field's fixed -30..30, and textured with texture\UDG.jpg instead of
// Field's siroiyuka.jpg. Kept as its own class rather than parameterizing
// Field itself, so nothing about the (already-working) stage 1 ground plane
// has to change.
class UndergroundFloor : public GameObject
{
private:

	ID3D11Buffer* m_VertexBuffer;

	ID3D11InputLayout* m_VertexLayout;
	ID3D11VertexShader* m_VertexShader;
	ID3D11PixelShader* m_PixelShader;

	ID3D11ShaderResourceView* m_Texture;

public:

	void Init()override;
	void Uninit()override;
	void Update()override;
	void Draw()override;

	// STEP51: a single plane spanning the whole underground map -- never
	// cull (see gameObject.h IsCullable(), and Field.h's identical override).
	bool IsCullable() const override { return false; }
};
