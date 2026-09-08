#pragma once

#include "gameObject.h"

// STEP9: purely decorative ceiling prop -- the downloaded fluorescent light
// fixture case/housing model (model\Case_Obj\Case_Obj\
// Fluorecent_Light_Case_LP.obj). See lightTube.h for the full story (same
// pattern, same missing-mtllib/usemtl problem, same fix shape) -- this is
// the case half of the pair, meant to be placed alongside a LightTube at
// roughly the same spot.
//
// Scale: the model's native units come out to ~18 units along its longest
// axis -- about 1/10th of LightTube_LP.obj's ~178. These are two separately
// downloaded packs, not a matched assembly, so that ratio may or may not be
// meaningful. The default scale in lightCase.cpp (0.1) is picked so this
// model's length roughly matches LightTube's default-scaled length (both
// land around ~1.8 game units), as a starting point for "case housing the
// tube" -- not a verified fit. Retune via SetScale()/SetPosition()/
// SetRotation() once both are visible in-game together.
class LightCase : public GameObject
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
