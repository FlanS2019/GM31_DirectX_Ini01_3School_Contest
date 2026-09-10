#pragma once

#include "gameObject.h"

// Decorative prop -- a pair of stacked wooden crates (model\Crate_2x.obj,
// two separate objects -- Crate_01_GEO/Crate_02_GEO -- baked into one file
// at a fixed relative offset, both sharing material "Crate_2x"). The
// downloaded asset had a `mtllib Crate_2x.mtl` reference but the matching
// .mtl (and any texture) never actually shipped with it -- same problem as
// WoodenPallet_Broken2/Leather_Stool, fixed the same way: a hand-written
// solid wood-brown Crate_2x.mtl (see model\Crate_2x.mtl).
//
// Scale: native units already read as real-world meters (each crate is
// roughly 1.2-1.3m wide, 0.42m tall) -- no extra scale needed. Both crates'
// own Y origin sits at (or just above) the floor already, so like
// Stool/WoodenPallet, m_Position is used directly with no pivot shift.
class Crate : public GameObject
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
