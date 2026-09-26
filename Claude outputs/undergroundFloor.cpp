//undergroundFloor.cpp
#include "undergroundFloor.h"
#include "main.h"
#include "renderer.h"

namespace
{
	// STEP02(stage2)-fix: covers the entrance room (X:[-6,6], Z:[-4,4]) and
	// the whole grid beyond it (X:[-18,18], Z:[4.3, 80.3] -- see
	// undergroundMap.cpp's kGridStartZ/kGridCols/kGridRows), with a bit of
	// margin on every side.
	const float kHalfX = 20.0f;
	const float kMinZ = -6.0f;
	const float kMaxZ = 84.0f;

	// Same tiling density as Field.cpp's (~6 world units per UV tile).
	const float kUvX = (kHalfX * 2.0f) / 6.0f;
	const float kUvZ = (kMaxZ - kMinZ) / 6.0f;
}

void UndergroundFloor::Init()
{
	VERTEX_3D vertex[4];

	vertex[0].Position = XMFLOAT3(-kHalfX, 0, kMaxZ);
	vertex[0].Normal = XMFLOAT3(0, 1, 0);
	vertex[0].Diffuse = XMFLOAT4(1, 1, 1, 1);
	vertex[0].TexCoord = XMFLOAT2(0, 0);

	vertex[1].Position = XMFLOAT3(kHalfX, 0, kMaxZ);
	vertex[1].Normal = XMFLOAT3(0, 1, 0);
	vertex[1].Diffuse = XMFLOAT4(1, 1, 1, 1);
	vertex[1].TexCoord = XMFLOAT2(kUvX, 0);

	vertex[2].Position = XMFLOAT3(-kHalfX, 0, kMinZ);
	vertex[2].Normal = XMFLOAT3(0, 1, 0);
	vertex[2].Diffuse = XMFLOAT4(1, 1, 1, 1);
	vertex[2].TexCoord = XMFLOAT2(0, kUvZ);

	vertex[3].Position = XMFLOAT3(kHalfX, 0, kMinZ);
	vertex[3].Normal = XMFLOAT3(0, 1, 0);
	vertex[3].Diffuse = XMFLOAT4(1, 1, 1, 1);
	vertex[3].TexCoord = XMFLOAT2(kUvX, kUvZ);

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
	LoadFromWICFile(L"texture\\UDG.jpg", WIC_FLAGS_NONE, &metadata, image);
	CreateShaderResourceView(Renderer::GetDevice(), image.GetImages(), image.GetImageCount(), metadata, &m_Texture);
}

void UndergroundFloor::Uninit()
{
	m_VertexBuffer->Release();
	m_VertexLayout->Release();
	m_VertexShader->Release();
	m_PixelShader->Release();
	m_Texture->Release();
}

void UndergroundFloor::Update()
{
}

void UndergroundFloor::Draw()
{
	Renderer::GetDeviceContext()->IASetInputLayout(m_VertexLayout);

	Renderer::GetDeviceContext()->VSSetShader(m_VertexShader, NULL, 0);
	Renderer::GetDeviceContext()->PSSetShader(m_PixelShader, NULL, 0);

	Renderer::GetDeviceContext()->PSSetShaderResources(0, 1, &m_Texture);//テクスチャをピクセルシェーダへバインド (t0)

	XMMATRIX world, scale, rot, trans;
	scale = XMMatrixScaling(m_Scale.x, m_Scale.y, m_Scale.z);//拡大率
	rot = XMMatrixRotationRollPitchYaw(m_Rotation.x, m_Rotation.y, m_Rotation.z);//回転角
	trans = XMMatrixTranslation(m_Position.x, m_Position.y, m_Position.z);//平行移動量
	world = scale * rot * trans;

	Renderer::SetWorldMatrix(world);

	MATERIAL material{};
	material.Diffuse = { 1.0f, 1.0f, 1.0f, 1.0f };
	material.TextureEnable = TRUE;
	Renderer::SetMaterial(material);

	UINT stride = sizeof(VERTEX_3D);
	UINT offset = 0;
	Renderer::GetDeviceContext()->IASetVertexBuffers(0, 1, &m_VertexBuffer, &stride, &offset);

	Renderer::GetDeviceContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

	Renderer::GetDeviceContext()->Draw(4, 0);
}
