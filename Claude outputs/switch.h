#pragma once

#include "gameObject.h"
#include "interactable.h"

class Door;

// STEP: 「鍵っぽい見た目にしてほしい」との要望で追加。以前はswitch.obj
// (key.objと同じ単なる箱、色だけ違う仮モデル)を表示していて、見た目が
// 「何もない箱」になってしまっていた。
//
// STEP16追記: 一度は鍵をかたどったモデル(model\Switch_Key.obj)にしたが、
// 実際の拾い物のKey(model\Key_Pickup.obj)と見た目がほぼ同じ「金色の鍵」
// になってしまい、「黄色い物体と鍵のオブジェクトが同じ動作に見える、
// 片方しか拾えないのがおかしい」という混同を招いた。SwitchはInteract()で
// m_TargetDoorを開けるだけの壁掛け機構であり、インベントリに入る拾い物
// ではない(Key/Itemとは別物)。そこで鍵の形をやめ、壁掛けレバースイッチの
// 形(model\Switch_Lever.obj -- 取り付け板+レバー柄+先端ノブの3ボックス
// 構成、テクスチャは手元に無いので単色プレースホルダー --
// model\Switch_Lever.mtl)に変更し、見た目からして「鍵ではない」と
// 分かるようにした。当たり判定・操作の仕組み(m_TargetDoorを開けるだけ)は
// 元のまま変更なし。
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
