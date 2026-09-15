#pragma once

#include "box.h"
#include "interactable.h"

class ModelRenderer;
class Audio;

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

	// STEP19: gimmick doors (opened only via a Switch calling Open()
	// directly) were still Interactable with m_RequiredKeyId's default of -1
	// ("no key needed, E alone opens it"), so the player could walk up and
	// press E on the door itself, skipping the switch entirely. This flag
	// lets Map.cpp mark a door as switch-only so CanInteract() below refuses
	// direct E-interact no matter what m_RequiredKeyId is -- Open() (what
	// Switch::Interact() calls) is a separate method and is unaffected.
	bool m_SwitchOnly = false;

	// STEP21: "ドアを開ける音" -- lives on Door (not Player) since it's tied
	// to a specific door's position/identity, unlike the shared pickup SE.
	Audio* m_OpenSE = nullptr;

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
	bool CanInteract() override { return !m_SwitchOnly && m_OpenT < 1.0f; } // nothing left once fully open; switch-only doors never take direct E-interact

	void SetRequiredKey(int keyId) { m_RequiredKeyId = keyId; }
	void SetSwitchOnly(bool switchOnly) { m_SwitchOnly = switchOnly; }

	void Open(); // STEP21: now plays m_OpenSE too -- see door.cpp. Was inline ("{ m_Open = true; }"); moved to the .cpp so both this AND Interact() share one place that can't fire the SE twice or forget it.

	bool IsOpen() const { return m_OpenT >= 1.0f; }

	void SetIsExit(bool isExit) { m_IsExit = isExit; }
};
