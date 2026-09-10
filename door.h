#pragma once

#include "box.h"
#include "interactable.h"

class ModelRenderer;

// STEP: 元は単なるbox.obj(壁と同じ見た目の箱)をスライドさせるだけの
// ドアだったが、Door_VAR01(実際の観音開きドア)を導入したのでシステムを
// 描画まわりだけ変更 -- 枠(Frame)+左扉(LeafLeft)+右扉(LeafRight)の3
// パーツを別々のワールド行列で描き、左右の扉だけ蝶番を軸に回転して開く。
// 当たり判定(m_Position/m_Scaleの意味・IsBlocking()の閾値)は元のBoxベースの
// ドアと完全に同じままなので、Player.cppの壁判定・Map.cppからの呼び出し
// 方は変わらない。Map.cpp側はSpawnDoorway()ヘルパー経由で生成する(枠の
// 実寸がセル幅より狭いので、両脇に袖壁を追加する処理とセットになっている)。
class Door : public Box, public Interactable
{
private:
	int m_RequiredKeyId = -1; // -1 = no key needed, 'E' alone opens it
	bool m_Open = false;      // true once opening has been triggered
	float m_OpenT = 0.0f;     // 0 = closed (leaves flush in the frame), 1 = fully open
	bool m_IsExit = false;
	bool m_ClearTriggered = false;

	// Box::Init()は呼ばない(box.objは要らない)ので、シェーダーもここで
	// 自前に読み込む -- door.cppのコメント参照。
	ID3D11InputLayout* m_VertexLayout = nullptr;
	ID3D11VertexShader* m_VertexShader = nullptr;
	ID3D11PixelShader* m_PixelShader = nullptr;

	ModelRenderer* m_FrameRenderer = nullptr;
	ModelRenderer* m_LeftLeafRenderer = nullptr;
	ModelRenderer* m_RightLeafRenderer = nullptr;

public:
	void Init()override;
	void Uninit()override;
	void Update()override;
	void Draw()override;

	bool IsBlocking() const override { return m_OpenT < 1.0f; }

	const char* GetInteractText() override;
	void Interact() override;
	bool CanInteract() override { return m_OpenT < 1.0f; } // nothing left to interact with once fully open

	void SetRequiredKey(int keyId) { m_RequiredKeyId = keyId; }

	void Open() { m_Open = true; }

	bool IsOpen() const { return m_OpenT >= 1.0f; }

	void SetIsExit(bool isExit) { m_IsExit = isExit; }
};
