#pragma once

#include "gameObject.h"

// Decorative prop -- a broken wooden pallet, converted from
// model\cgtrader_optimized_WoodenPallet_Broken2.fbx via `assimp export`
// (this project's model loader only understands .obj, not .fbx directly).
// The converted mesh lives at model\WoodenPallet_Broken2.obj, alongside a
// hand-written model\WoodenPallet_Broken2.mtl.
//
// The source FBX has zero embedded textures (confirmed with `assimp info`),
// and assimp's auto-exported .mtl pointed at external texture files that
// don't exist in this project and whose path contains a space character --
// which would break this project's fscanf("%s", ...)-based .mtl parser
// (it reads one whitespace-delimited token per field). Replaced with a
// hand-written .mtl using a plain solid wood-brown color instead.
//
// Scale: native units already come out to a real-world size (~1.2m x
// 1.03m x 0.16m -- an ordinary pallet footprint), so no extra scale is
// needed. The base sits at Y~0 like LightCase/Stool. Unlike those, though,
// the source FBX's own pivot is NOT centered on the mesh's X/Z footprint
// (model-space X runs ~2.39..3.59, nowhere near 0) -- m_Position is still
// used directly with no pivot shift (see Draw()'s comment for why
// re-centering it was tried and reverted), so callers placing this prop
// need to account for that raw offset themselves if they care about exact
// footprint alignment. SpawnLeaningPallet in Map.cpp is the only caller
// today and handles this.
class WoodenPallet : public GameObject
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
