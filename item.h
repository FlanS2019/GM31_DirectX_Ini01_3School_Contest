#pragma once

#include "gameObject.h"
#include "interactable.h"

class ModelRenderer;

class Item : public GameObject, public Interactable
{
private:
	int m_ItemId = 0;
	const char* m_DisplayName = "アイテム";
	bool m_Collected = false;

	char m_PromptBuf[64]{}; // GetInteractText() builds "E ○○を拾う" into this

	ID3D11InputLayout* m_VertexLayout = nullptr;
	ID3D11VertexShader* m_VertexShader = nullptr;
	ID3D11PixelShader* m_PixelShader = nullptr;

	// STEP: 「拾う3つの物資(写真/手紙/金属部品)のモデルを作ってほしい」との
	// 要望で追加。以前はItemIdに関係なく全部key.obj(仮モデル)を表示して
	// いたが、Init()の時点ではまだMap.cppからSetItemId()が呼ばれておらず
	// IDが確定していない(コンストラクト直後→AddComponent<ModelRenderer>
	// →その後で呼び出し側がSetItemId()する順番)ので、モデルの選択は
	// SetItemId()側で行う。そのためInit()で作ったModelRendererをここに
	// 保持しておく -- item.cpp参照。
	ModelRenderer* m_ModelRenderer = nullptr;

public:
	void Init() override;
	void Uninit() override;
	void Update() override;
	void Draw() override;

	// Map.cpp calls these right after AddGameObject<Item>().
	void SetItemId(int id); // now also (re)loads the item-specific model -- see item.cpp
	void SetDisplayName(const char* name) { m_DisplayName = name; }

	const char* GetInteractText() override;
	void Interact() override;
	bool CanInteract() override { return !m_Collected; }
};
