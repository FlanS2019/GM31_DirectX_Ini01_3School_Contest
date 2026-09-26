#pragma once

#include "gameObject.h"

// STEP02(stage2)-detail: decorative storage-room shelving (model\shelfRack.obj
// -- procedurally generated, see that file's header comment). Purely visual,
// same as Crate/Stool/WoodenPallet.
class ShelfRack : public GameObject
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
