#pragma once

#include "gameObject.h"

// Decorative prop -- ivy leaves only (model\Ivy.obj), no backing wall.
// Split out of the original model\boston+ivy.obj download, which combined
// two "backing box" meshes (material wire_228184153) with the dense
// ivy-leaf mesh (material wire_141007058) -- dropped the backing box
// entirely and kept only the ivy-leaf geometry, since this prop is meant
// to hang on top of the game's own wall geometry, not bring its own wall.
// No texture/mtl shipped with the download, so (same fix shape as
// WoodenPallet_Broken2/Leather_Stool/Crate_2x) the material is a plain
// solid green (model\ivy.mtl).
//
// Heavy mesh (roughly 139k triangles) -- fine to draw, but expect a brief
// hitch the first time Map::Init() loads it.
//
// Scale unchanged from the combined model: native units read as
// centimeters (3ds Max's usual default), so 0.01 brings it to game-world
// meters. Vertex coordinates were NOT recentered during the split (kept
// exactly as exported), so this drops in as a straight swap for the old
// boston+ivy.obj load -- Map.cpp's existing SpawnIvy() position/rotation
// tuning still lines up unchanged.
class Ivy : public GameObject
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
