#pragma once

#include "gameObject.h"

// STEP9: purely decorative ceiling prop -- the downloaded fluorescent tube
// model (model\Tube_Obj\Tube_Obj\LightTube_LP.obj). Not an Interactable, not
// a Box (no collision) -- just something to look at, same "one GameObject +
// one ModelRenderer component" pattern as Item/Box/Key.
//
// The .obj as downloaded had NO mtllib/usemtl lines at all (common for
// asset-pack exports meant to be re-textured by hand) -- ModelRenderer::
// LoadObj() counts subsets by counting "usemtl" lines, so with zero of them
// SubsetNum came out 0 and nothing ever drew. Fixed by adding a
// "mtllib LightTube_LP.mtl" line near the top of the .obj and a
// "usemtl Tube" line right before its first "f" line, plus the new
// LightTube_LP.mtl itself (references LightTube_LP_Color.png -- the _ARM/
// _Normal/_Em maps from the same download aren't usable without shader
// changes, since this project's shader is unlit + single-texture).
//
// Scale: the model's native units come out to ~178 units along its longest
// axis. Assumed to be centimeters (a ~1.8m tube is plausible), so the
// default scale in lightTube.cpp (0.01) targets "1 game unit == 1 meter",
// matching the rest of this project's world (CELL_SIZE/WALL_HEIGHT etc).
// This is a guess, not a measurement -- there's no way to preview the actual
// render from here, so retune via SetScale()/SetPosition()/SetRotation()
// (all inherited from GameObject) once it's visible in-game.
class LightTube : public GameObject
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
