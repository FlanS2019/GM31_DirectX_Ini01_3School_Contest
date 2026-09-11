#pragma once

#include "gameObject.h"

// STEP: 「血っぽいやつで廃墟っぽさを増やしたい」との要望で追加。
// StainDirtと構造は同じ(平らなデカール的プレースホルダー、Y=0.01)で、
// 中央の血だまりに加えて周りに飛び散った小さい三角形(飛沫)を何個か
// くっつけてある -- model\Stain_Blood.obj / model\Stain_Blood.mtl参照。
// あまり数を置きすぎるとくどいので、StainDirtより控えめな個数を推奨。
class StainBlood : public GameObject
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
