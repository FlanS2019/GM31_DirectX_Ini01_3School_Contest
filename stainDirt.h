#pragma once

#include "gameObject.h"

// STEP: 「床の汚れで廃墟っぽさを増やしたい」との要望で追加。床にへばり
// ついた汚れのデカール的な平面(model\Stain_Dirt.obj) -- Crate/Ivyと同じ
// 理由でテクスチャ無しの単色プレースホルダー(model\Stain_Dirt.mtl)。
// モデル自体がローカルY=0.01(床メッシュとのZファイティング防止に少し
// 浮かせてある)の真っ平らな不整形パッチなので、SetPosition()はそのまま
// 床の高さ(Y=0)でOK、SetScale()で1個ずつ大きさを変えるとバラつきが出る。
class StainDirt : public GameObject
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
