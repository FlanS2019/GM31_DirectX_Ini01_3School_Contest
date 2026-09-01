//polygon2d.cpp
#include "polygon2d.h"
#include "main.h"
#include "renderer.h"

void Polygon2D::Init(float x,float y, float width, float Height, const WCHAR* TextureName)
{
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
	Renderer::CreateVertexShader(&m_VertexShader, &m_VertexLayout, "shaderunlitTextureVS.cso");
	Renderer::CreatePixelShader(&m_PixelShader, "shaderunlitTexturePS.cso");

	//テクスチャの作成
	TexMetadata metadata{};
	ScratchImage image{};
	LoadFromWICFile(TextureName, WIC_FLAGS_NONE, &metadata, image);
	CreateShaderResourceView(Renderer::GetDevice(), image.GetImages(), image.GetImageCount(), metadata, &m_Texture);
}

void Polygon2D::Uninit()
{
	m_VertexBuffer->Release();
	m_VertexLayout->Release();
	m_VertexShader->Release();
	m_PixelShader->Release();
	m_Texture->Release();
}

void Polygon2D::Update()
{
}

void Polygon2D::Draw()
{
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
}