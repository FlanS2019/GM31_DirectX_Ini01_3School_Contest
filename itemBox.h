#pragma once

#include "gameObject.h"
#include "interactable.h"

class ItemBox : public GameObject, public Interactable
{
private:
	int m_RequiredItemIds[3] = { -1, -1, -1 };
	int m_FinalKeyId = -1;
	bool m_Opened = false;

	ID3D11InputLayout* m_VertexLayout = nullptr;
	ID3D11VertexShader* m_VertexShader = nullptr;
	ID3D11PixelShader* m_PixelShader = nullptr;

	bool AllItemsCollected() const;

public:
	void Init() override;
	void Uninit() override;
	void Update() override;
	void Draw() override;

	// Map.cpp calls these right after AddGameObject<ItemBox>().
	void SetRequiredItemIds(int a, int b, int c) { m_RequiredItemIds[0] = a; m_RequiredItemIds[1] = b; m_RequiredItemIds[2] = c; }
	void SetFinalKeyId(int id) { m_FinalKeyId = id; }

	const char* GetInteractText() override;
	void Interact() override;
	bool CanInteract() override { return !m_Opened; }
};