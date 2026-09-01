// explosion.cpp

#include "explosion.h"
#include "main.h"
#include "renderer.h"
#include "manager.h"
#include "camera.h"

void Explosion::Init()
{
	m_Layer = 5;

	VERTEX_3D vertex[4];

	/* 頂点のY座標に高さを設定し、板ポリゴンとしての面積を持たせます。 */

	vertex[0].Position = XMFLOAT3(-4.0f, 8.0f, 0.0f); /* 左上。 */
	vertex[0].Normal = XMFLOAT3(0.0f, 0.0f, -1.0f);
	vertex[0].Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	vertex[0].TexCoord = XMFLOAT2(0, 0);

	vertex[1].Position = XMFLOAT3(4.0f, 8.0f, 0.0f);  /* 右上。 */
	vertex[1].Normal = XMFLOAT3(0.0f, 0.0f, -1.0f);
	vertex[1].Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	vertex[1].TexCoord = XMFLOAT2(1, 0);

	vertex[2].Position = XMFLOAT3(-4.0f, 0.0f, 0.0f); /* 左下。 */
	vertex[2].Normal = XMFLOAT3(0.0f, 0.0f, -1.0f);
	vertex[2].Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	vertex[2].TexCoord = XMFLOAT2(0, 1);

	vertex[3].Position = XMFLOAT3(4.0f, 0.0f, 0.0f);  /* 右下。 */
	vertex[3].Normal = XMFLOAT3(0.0f, 0.0f, -1.0f);
	vertex[3].Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	vertex[3].TexCoord = XMFLOAT2(1, 1);

	/* 頂点バッファの作成（動的にUVを書き換えるため DYNAMIC）。 */
	D3D11_BUFFER_DESC bd{};
	bd.Usage = D3D11_USAGE_DYNAMIC; // 動的に Map して更新する
	bd.ByteWidth = sizeof(VERTEX_3D) * 4;
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	bd.MiscFlags = 0;
	bd.StructureByteStride = 0;

	D3D11_SUBRESOURCE_DATA sd{};
	sd.pSysMem = vertex;

	//textureの読み込み
	TexMetadata metadata{};
	ScratchImage image{};
	LoadFromWICFile(L"texture\\explosion.png", WIC_FLAGS_NONE, &metadata, image);
	CreateShaderResourceView(Renderer::GetDevice(), image.GetImages(), image.GetImageCount(), metadata, &m_Texture);
	assert(m_Texture);


	Renderer::GetDevice()->CreateBuffer(&bd, &sd, &m_VertexBuffer);

	/* シェーダーの作成。 */

	Renderer::CreateVertexShader(&m_VertexShader, &m_VertexLayout, "shader\\unlitTextureVS.cso");
	Renderer::CreatePixelShader(&m_PixelShader, "shader\\unlitTexturePS.cso");

	m_Frame = 0;
}

void Explosion::Uninit()
{
	if (m_VertexBuffer) { m_VertexBuffer->Release(); m_VertexBuffer = nullptr; }
	if (m_VertexLayout) { m_VertexLayout->Release(); m_VertexLayout = nullptr; }
	if (m_VertexShader) { m_VertexShader->Release(); m_VertexShader = nullptr; }
	if (m_PixelShader) { m_PixelShader->Release(); m_PixelShader = nullptr; }
	if (m_Texture) { m_Texture->Release(); m_Texture = nullptr; } /* テクスチャの解放。 */
}

void Explosion::Update()
{
	m_Frame++;

	if(m_Frame > 16) /* 16フレームで消える。 */
	{
		SetDestroy(true);
	}
}

void Explosion::Draw()
{
	Renderer::GetDeviceContext()->IASetInputLayout(m_VertexLayout);
	Renderer::GetDeviceContext()->VSSetShader(m_VertexShader, NULL, 0);
	Renderer::GetDeviceContext()->PSSetShader(m_PixelShader, NULL, 0);
	Renderer::GetDeviceContext()->PSSetShaderResources(0, 1, &m_Texture); /* テクスチャをピクセルシェーダーへバインド。 */

	// 半透明描画のためブレンドと深度設定を調整
	Renderer::SetATCEnable(true);
	Renderer::SetDepthEnable(false);


	D3D11_MAPPED_SUBRESOURCE msr{};
	Renderer::GetDeviceContext()->Map(m_VertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &msr);

	float tx = 1.0f / 4.0f * (m_Frame % 4); /* テクスチャのX方向のオフセット。 */
	float ty = 1.0f / 4.0f * (m_Frame / 4); /* テクスチャのY方向のオフセット。 */
	float tw = 1.0f / 4.0f; /* テクスチャのX方向の幅。 */
	float th = 1.0f / 4.0f; /* テクスチャのY方向の幅。 */

	VERTEX_3D* vertex = (VERTEX_3D*)msr.pData;
	vertex[0].Position = XMFLOAT3(-4.0f, 8.0f, 0.0f); /* 左上。 */
	vertex[0].Normal = XMFLOAT3(0.0f, 0.0f, -1.0f);
	vertex[0].Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	vertex[0].TexCoord = XMFLOAT2(tx, ty);

	vertex[1].Position = XMFLOAT3(4.0f, 8.0f, 0.0f);  /* 右上。 */
	vertex[1].Normal = XMFLOAT3(0.0f, 0.0f, -1.0f);
	vertex[1].Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	vertex[1].TexCoord = XMFLOAT2(tx + tw, ty);

	vertex[2].Position = XMFLOAT3(-4.0f, 0.0f, 0.0f); /* 左下。 */
	vertex[2].Normal = XMFLOAT3(0.0f, 0.0f, -1.0f);
	vertex[2].Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	vertex[2].TexCoord = XMFLOAT2(tx, ty + th);

	vertex[3].Position = XMFLOAT3(4.0f, 0.0f, 0.0f);  /* 右下。 */
	vertex[3].Normal = XMFLOAT3(0.0f, 0.0f, -1.0f);
	vertex[3].Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	vertex[3].TexCoord = XMFLOAT2(tx + tw, ty + th);

	Renderer::GetDeviceContext()->Unmap(m_VertexBuffer, 0);
	/* ビルボード行列の作成（Y軸ビルボード）。 */

	Camera* camera = Manager::GetGameObject<Camera>();
	XMMATRIX view = camera->GetViewMatrix();
	XMMATRIX invView = XMMatrixInverse(NULL, view);

	/* 平行移動成分をクリア。 */

	invView.r[3].m128_f32[0] = 0.0f;
	invView.r[3].m128_f32[1] = 0.0f;
	invView.r[3].m128_f32[2] = 0.0f;

	XMMATRIX world, scale, trans;

	scale = XMMatrixScaling(m_Scale.x, m_Scale.y, m_Scale.z); /* 拡大縮小。 */
	trans = XMMatrixTranslation(m_Position.x, m_Position.y, m_Position.z); /* 平行移動。 */
	world = scale * invView * trans;

	Renderer::SetWorldMatrix(world);

	MATERIAL material{};
	material.Diffuse = { 1.0f, 1.0f, 1.0f, 1.0f };
	material.TextureEnable = TRUE; /* テクスチャ有効化。 */

	Renderer::SetMaterial(material);
	UINT stride = sizeof(VERTEX_3D);
	UINT offset = 0;
	Renderer::GetDeviceContext()->IASetVertexBuffers(0, 1, &m_VertexBuffer, &stride, &offset);
	Renderer::GetDeviceContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
	Renderer::GetDeviceContext()->Draw(4, 0);

	// 状態を復帰
	Renderer::SetDepthEnable(true);
	Renderer::SetATCEnable(false);
}