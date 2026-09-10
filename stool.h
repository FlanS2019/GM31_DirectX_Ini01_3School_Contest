#pragma once

#include "gameObject.h"

// Decorative prop -- a leather stool (model\Leather_Stool.obj). The
// downloaded asset had NO .mtl at all (no mtllib, no usemtl anywhere in the
// .obj) -- a step further than lightCase.h/lightTube.h's problem, which at
// least had a mtllib line but no usemtl. Same fix shape either way: without
// a usemtl before the face list, ModelRenderer::LoadObj counts zero subsets,
// so the model loads fine but Draw() has zero subsets to iterate over and
// draws nothing. Fixed by inserting `mtllib Leather_Stool.mtl` near the top
// and `usemtl Leather` right before the face list, plus a hand-written
// Leather_Stool.mtl (no source texture exists for this asset, so it's a
// plain solid leather-brown color).
//
// Scale: native units come out to ~455 x 687 x 433 (x/y/z). These read as
// millimeters (a ~68.7cm-tall, ~45cm-wide stool is a normal real-world
// size), so scale 0.001 (a literal mm->m conversion) brings it to
// game-world meters at that real-world size. (Was 0.0007, which undersized
// it by about 30% -- looked too small next to the rest of the scene.) The
// model's local origin sits essentially at its base (min Y ~ -0.26 out of a
// 687 range), so like LightCase/LightTube it's positioned directly with no
// separate pivot shift.
class Stool : public GameObject
{
private:
	ID3D11InputLayout* m_VertexLayout = nullptr;
	ID3D11VertexShader* m_VertexShader = nullptr;
	ID3D11PixelShader* m_PixelShader = nullptr;

public:
	void Init() override;
	void Uninit() override;
	void Draw() override;
};
