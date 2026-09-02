#pragma once

#include "gameObject.h"

class Door;

// STEP2 gimmick: walk up, press 'E', it opens whichever Door it's wired to
// (no key needed). Doesn't block movement -- same reasoning as Key.
class Switch : public GameObject
{
private:
	Door* m_TargetDoor = nullptr;
	bool m_Used = false;

	ID3D11InputLayout* m_VertexLayout = nullptr;
	ID3D11VertexShader* m_VertexShader = nullptr;
	ID3D11PixelShader* m_PixelShader = nullptr;

public:
	void Init()override;
	void Uninit()override;
	void Update()override;
	void Draw()override;

	// Map.cpp calls this right after creating both the Switch and the Door
	// it should open, to wire them together.
	void SetTargetDoor(Door* door) { m_TargetDoor = door; }
};
