#pragma once

#include "gameObject.h"
#include "interactable.h"

class Door;

class Switch : public GameObject, public Interactable
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

	const char* GetInteractText() override { return "E ëÄçÏ"; }
	void Interact() override;
	bool CanInteract() override { return !m_Used; }

	void SetTargetDoor(Door* door) { m_TargetDoor = door; }
};