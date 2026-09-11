#pragma once

#include "gameObject.h"

// STEP: 「床の汚れ・血・木の棒とかで廃墟っぽさを増やしたい」との要望で
// 追加。木の棒/板きれの散らかりもの -- Crate/Ivyと同じ理由(テクスチャが
// 手元に無い)で単色プレースホルダー(model\Debris_Stick.mtl)。
// model\Debris_Stick.objはローカルY=0が下面になるようにモデリングして
// あるので、床にそのまま置くだけ(SetPosition()のYは0でOK、床にめり込ま
// ない)。横に転がってる想定でSetRotation()のXは基本0のまま、Yだけ回して
// 向きを変える。
class Debris : public GameObject
{
private:
	ID3D11InputLayout* m_VertexLayout = nullptr;
	ID3D11VertexShader* m_VertexShader = nullptr;
	ID3D11PixelShader* m_PixelShader = nullptr;

public:
	void Init() override;
	void Uninit() override;
	void Draw() override;
};
