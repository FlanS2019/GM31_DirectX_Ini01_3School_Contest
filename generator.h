#pragma once

#include "gameObject.h"

// STEP02(stage2)-detail: decorative generator model for the underground's
// generator room (model\generator.obj -- procedurally generated, see that
// file's header comment). Purely visual for now, same as Crate/Stool --
// STEP08 (per the spec's construction-STEP table) is what turns this into
// something the player actually interacts with (fuse insertion, E-key
// operate); wiring that up is out of scope here.
class Generator : public GameObject
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
