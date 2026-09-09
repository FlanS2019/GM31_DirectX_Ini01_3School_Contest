#pragma once

#include "gameObject.h"
#include "interactable.h"

class Item : public GameObject, public Interactable
{
private:
	int m_ItemId = 0;
	const char* m_DisplayName = "ƒAƒCƒeƒ€";
	bool m_Collected = false;

	char m_PromptBuf[64]{}; // GetInteractText() builds "E ››‚ğE‚¤" into this

	ID3D11InputLayout* m_VertexLayout = nullptr;
	ID3D11VertexShader* m_VertexShader = nullptr;
	ID3D11PixelShader* m_PixelShader = nullptr;

public:
	void Init() override;
	void Uninit() override;
	void Update() override;
	void Draw() override;

	// Map.cpp calls these right after AddGameObject<Item>().
	void SetItemId(int id) { m_ItemId = id; }
	void SetDisplayName(const char* name) { m_DisplayName = name; }

	const char* GetInteractText() override;
	void Interact() override;
	bool CanInteract() override { return !m_Collected; }
};