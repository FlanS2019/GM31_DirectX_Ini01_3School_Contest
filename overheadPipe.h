#pragma once

#include "gameObject.h"

// STEP02(stage2)-detail: decorative ceiling-level pipe run (model\overheadPipe.obj
// -- procedurally generated, see that file's header comment), echoing the
// reference photo's overhead ductwork. Purely visual, same as Crate/Stool.
// Unlike the floor-based props, this one's local origin is the pipes' own
// centerline, not a floor-level base -- callers set m_Position.y near a
// ceiling and use m_Rotation.y to run it along world X (0) or world Z
// (PIDIV2), same convention as Map.cpp's SpawnIvy wallYRotation.
class OverheadPipe : public GameObject
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
