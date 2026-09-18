//polygon2d.cpp
#include "polygon2d.h"
#include "main.h"
#include "renderer.h"

void Polygon2D::Init(float x,float y, float width, float Height, const WCHAR* TextureName)
{
	m_Layer = 9;

	// STEP40: Draw()が毎フレームこの値で頂点バッファを作り直すので保存しておく。
	m_X = x;
	m_Y = y;
	m_Width = width;
	m_Height = Height;

	VERTEX_3D vertex[4];

	//vertex[0].Position = XMFLOAT3(0.0f, 0.0f, 0.0f);
	//vertex[0].Normal = XMFLOAT3(0.0f, 0.0f, -1.0f);
	//vertex[0].Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	//vertex[0].TexCoord = XMFLOAT2(0.0f, 0.0f);

	//vertex[1].Position = XMFLOAT3(200.0f, 0.0f, 0.0f);
	//vertex[1].Normal = XMFLOAT3(0.0f, 0.0f, 0.0f);
	//vertex[1].Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	//vertex[1].TexCoord = XMFLOAT2(1.0f, 0.0f);

	//vertex[2].Position = XMFLOAT3(0.0f, 200.0f, 0.0f);
	//vertex[2].Normal = XMFLOAT3(0.0f, 0.0f, 0.0f);
	//vertex[2].Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	//vertex[2].TexCoord = XMFLOAT2(0.0f, 1.0f);

	//vertex[3].Position = XMFLOAT3(200.0f, 200.0f, 0.0f);
	//vertex[3].Normal = XMFLOAT3(0.0f, 0.0f, 0.0f);
	//vertex[3].Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	//vertex[3].TexCoord = XMFLOAT2(1.0f, 1.0f);

	vertex[0].Position = XMFLOAT3(x, y, 0.0f);
	vertex[0].Normal = XMFLOAT3(0.0f, 0.0f, -1.0f);
	vertex[0].Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	vertex[0].TexCoord = XMFLOAT2(0.0f, 0.0f);

	vertex[1].Position = XMFLOAT3(x + width, y, 0.0f);
	vertex[1].Normal = XMFLOAT3(0.0f, 0.0f, 0.0f);
	vertex[1].Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	vertex[1].TexCoord = XMFLOAT2(1.0f, 0.0f);

	vertex[2].Position = XMFLOAT3(x ,y + Height, 0.0f);
	vertex[2].Normal = XMFLOAT3(0.0f, 0.0f, 0.0f);
	vertex[2].Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	vertex[2].TexCoord = XMFLOAT2(0.0f, 1.0f);

	vertex[3].Position = XMFLOAT3(x + width, y + Height, 0.0f);
	vertex[3].Normal = XMFLOAT3(0.0f, 0.0f, 0.0f);
	vertex[3].Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	vertex[3].TexCoord = XMFLOAT2(1.0f, 1.0f);

	//頂点バッファの作成
	D3D11_BUFFER_DESC bd{};
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(VERTEX_3D) * 4;
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = 0;//CPUからアクセスしない

	D3D11_SUBRESOURCE_DATA sd{};
	sd.pSysMem = vertex;
	Renderer::GetDevice()->CreateBuffer(&bd, &sd, &m_VertexBuffer);

	//シェーダーの作成
	Renderer::CreateVertexShader(&m_VertexShader, &m_VertexLayout, "shader\\unlitTextureVS.cso");
	Renderer::CreatePixelShader(&m_PixelShader, "shader\\unlitTexturePS.cso");

	//テクスチャの作成
	TexMetadata metadata{};
	ScratchImage image{};
	m_Texture = nullptr;
	HRESULT hr = LoadFromWICFile(TextureName, WIC_FLAGS_NONE, &metadata, image);
	if (SUCCEEDED(hr))
	{
		hr = CreateShaderResourceView(Renderer::GetDevice(), image.GetImages(), image.GetImageCount(), metadata, &m_Texture);
	}
	if (FAILED(hr))
	{
		// STEP12: image1.png failed to load/convert -- leaving m_Texture as
		// garbage here used to hand the GPU driver a wild pointer via
		// PSSetShaderResources in Draw(), which is exactly the kind of bug
		// that crashes deep inside nvwgf2umx.dll instead of failing cleanly.
		// Draw() below now skips binding/drawing entirely when this is null.
		m_Texture = nullptr;
	}
}

void Polygon2D::Uninit()
{
	if (m_VertexBuffer) m_VertexBuffer->Release();
	if (m_VertexLayout) m_VertexLayout->Release();
	if (m_VertexShader) m_VertexShader->Release();
	if (m_PixelShader) m_PixelShader->Release();
	if (m_Texture) m_Texture->Release();
}

void Polygon2D::Update()
{
}

void Polygon2D::Draw()
{
	if (!m_Texture) return; // STEP12: texture failed to load in Init() -- nothing safe to draw

	// STEP40: 現在のm_Alphaを反映した頂点を作り直してGPU側のバッファを更新する。
	// Usage=D3D11_USAGE_DEFAULTなのでMap/Unmap(DYNAMIC用)は使えず、UpdateSubresource()で
	// 書き換える(タイトルロゴ/スプラッシュロゴのフェードイン・アウトはこの仕組みが
	// 無いと実現できない)。
	VERTEX_3D vertex[4];

	vertex[0].Position = XMFLOAT3(m_X, m_Y, 0.0f);
	vertex[0].Normal = XMFLOAT3(0.0f, 0.0f, -1.0f);
	vertex[0].Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, m_Alpha);
	vertex[0].TexCoord = XMFLOAT2(0.0f, 0.0f);

	vertex[1].Position = XMFLOAT3(m_X + m_Width, m_Y, 0.0f);
	vertex[1].Normal = XMFLOAT3(0.0f, 0.0f, 0.0f);
	vertex[1].Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, m_Alpha);
	vertex[1].TexCoord = XMFLOAT2(1.0f, 0.0f);

	vertex[2].Position = XMFLOAT3(m_X, m_Y + m_Height, 0.0f);
	vertex[2].Normal = XMFLOAT3(0.0f, 0.0f, 0.0f);
	vertex[2].Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, m_Alpha);
	vertex[2].TexCoord = XMFLOAT2(0.0f, 1.0f);

	vertex[3].Position = XMFLOAT3(m_X + m_Width, m_Y + m_Height, 0.0f);
	vertex[3].Normal = XMFLOAT3(0.0f, 0.0f, 0.0f);
	vertex[3].Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, m_Alpha);
	vertex[3].TexCoord = XMFLOAT2(1.0f, 1.0f);

	Renderer::GetDeviceContext()->UpdateSubresource(m_VertexBuffer, 0, nullptr, vertex, 0, 0);

	// STEP42: unlitTexturePS.hlslは名前に反してLight.Enableがtrueならシーンのライティング(light.cppで
	// Ambient=0.02程度の暗い値)をそのまま掛けてしまい、タイトルロゴやグレーフィルターなどの
	// UI用Polygon2Dがほぼ見えなくなっていた原因だった。この描画だけLightを一時的に無効化し、
	// 描画後に即座に元の値に戻す(他の3Dオブジェクトの照明には一切影響しない)。
	LIGHT savedLight = Renderer::GetLight();
	LIGHT unlitLight{}; // 全フィールドゼロ初期化 -- Enable=falseも含む
	Renderer::SetLight(unlitLight);

	Renderer::GetDeviceContext()->IASetInputLayout(m_VertexLayout);

	Renderer::GetDeviceContext()->VSSetShader(m_VertexShader, NULL, 0);
	Renderer::GetDeviceContext()->PSSetShader(m_PixelShader, NULL, 0);

	// テクスチャをピクセルシェーダへバインド (t0)
	Renderer::GetDeviceContext()->PSSetShaderResources(0, 1, &m_Texture);

	Renderer::SetWorldViewProjection2D();
	XMMATRIX world ,scale, rot, trans;
	scale = XMMatrixScaling(1.0f, 1.0f, 1.0f);//拡大率
	rot = XMMatrixRotationRollPitchYaw(0.0f, 0.0f, 0.0f);//回転角
	trans = XMMatrixTranslation(0.0f, 0.0f, 0.0f);//平行移動量
	world = scale * rot * trans;

	Renderer::SetWorldMatrix(world);

	MATERIAL material{};
	material.Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	material.TextureEnable = TRUE;
	Renderer::SetMaterial(material);

	UINT stride = sizeof(VERTEX_3D);
	UINT offset = 0;
	Renderer::GetDeviceContext()->IASetVertexBuffers(0, 1, &m_VertexBuffer, &stride, &offset);

	Renderer::GetDeviceContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

	Renderer::GetDeviceContext()->Draw(4, 0);

	Renderer::SetLight(savedLight); // STEP42: restore -- see note above
}