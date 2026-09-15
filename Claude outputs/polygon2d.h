#pragma once
#include <d3d11.h>
#include "gameObject.h"

class Polygon2D : public GameObject
{
private:

	ID3D11Buffer* m_VertexBuffer;

	ID3D11InputLayout* m_VertexLayout;
	ID3D11VertexShader* m_VertexShader;
	ID3D11PixelShader* m_PixelShader;

	ID3D11ShaderResourceView* m_Texture;

	// STEP40: タイトルロゴ/スプラッシュロゴのフェードイン・アウトで使う。Init()に渡された位置・サイズを保持しておき、Draw()のたびに現在のm_Alphaで頂点バッファを作り直す
	// (unlitTexturePS.hlslはMaterial.Diffuseではなく頂点側のIn.Diffuse.wしか見ていないため、Material定数バッファ経由では
	// フェードできない -- shader\unlitTexturePS.hlsl / shader\common.hlsl参照)。
	float m_X = 0.0f;
	float m_Y = 0.0f;
	float m_Width = 0.0f;
	float m_Height = 0.0f;
	float m_Alpha = 1.0f;

public:
	void Init()override {};
	void Init(float x, float y, float width, float Height, const WCHAR* TextureName);
	void Uninit()override;
	void Update()override;
	void Draw()override;

	// STEP40: 0=完全に透明、1=不透明。呼び出し側(TitleLogo/SplashLogo)が毎フレームUpdate()内で
	// セットし、Draw()が頂点バッファのDiffuse.wをUpdateSubresource()で書き換える。
	void SetAlpha(float alpha) { m_Alpha = alpha; }
	float GetAlpha() const { return m_Alpha; }
};
