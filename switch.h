#pragma once

#include "gameObject.h"
#include "interactable.h"

class Door;

// STEP: 「鍵っぽい見た目にしてほしい」との要望で追加。以前はswitch.obj
// (key.objと同じ単なる箱、色だけ違う仮モデル)を表示していて、見た目が
// 「何もない箱」になってしまっていた。今回、鍵をかたどったモデル
// (model\Switch_Key.obj -- 軸+ギザギザの歯+輪っかのシンプルな鍵の形、
// テクスチャは手元に無いので他の新規プロップと同じく単色のプレース
// ホルダー -- model\Switch_Key.mtl)に差し替えた。当たり判定・操作の
// 仕組み(m_TargetDoorを開けるだけ)は元のまま変更なし。
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

	const char* GetInteractText() override { return "E 操作"; }
	void Interact() override;
	bool CanInteract() override { return !m_Used; }

	void SetTargetDoor(Door* door) { m_TargetDoor = door; }
};
