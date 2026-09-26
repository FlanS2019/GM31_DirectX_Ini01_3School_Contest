#pragma once

#include "gameObject.h"

// STEP02(stage2)-detail: decorative industrial pump/motor unit (model\pumpUnit.obj
// -- procedurally generated, see that file's header comment), echoing the
// reference photo's green pump units. Purely visual, same as Crate/Stool/Generator.
class PumpUnit : public GameObject
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
