#pragma once

#include "gameObject.h"

// STEP2: a walk-up pickup. Doesn't block movement (plain GameObject, not a
// Box -- it must never enter Player.cpp's wall-collision loop), just waits
// for the player to get close and adds itself to their key inventory.
class Key : public GameObject
{
private:
	int m_KeyId = 0;

	ID3D11InputLayout* m_VertexLayout = nullptr;
	ID3D11VertexShader* m_VertexShader = nullptr;
	ID3D11PixelShader* m_PixelShader = nullptr;

public:
	void Init()override;
	void Uninit()override;
	void Update()override;
	void Draw()override;

	// Map.cpp calls this right after AddGameObject<Key>() to say which
	// Door(s) it should unlock (matched against Door::SetRequiredKey's id).
	void SetKeyId(int id) { m_KeyId = id; }
};
