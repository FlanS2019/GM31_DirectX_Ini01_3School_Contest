//grass.cpp

#include "grass.h"
#include "main.h"
#include "renderer.h"
#include "manager.h"
#include "camera.h"

void Grass::Init()

{

	VERTEX_3D vertex[4];

	/* 頂点のY座標に高さを設定し、板ポリゴンとしての面積を持たせます。 */

	vertex[0].Position = XMFLOAT3(-4.0f, 8.0f, 0.0f); /* 左上。 */

	vertex[0].Normal = XMFLOAT3(0.0f, 0.0f, -1.0f);

	vertex[0].Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);

	vertex[0].TexCoord = XMFLOAT2(0.0f, 0.0f);

	vertex[1].Position = XMFLOAT3(4.0f, 8.0f, 0.0f);  /* 右上。 */

	vertex[1].Normal = XMFLOAT3(0.0f, 0.0f, -1.0f);

	vertex[1].Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);

	vertex[1].TexCoord = XMFLOAT2(1.0f, 0.0f);

	vertex[2].Position = XMFLOAT3(-4.0f, 0.0f, 0.0f); /* 左下。 */

	vertex[2].Normal = XMFLOAT3(0.0f, 0.0f, -1.0f);

	vertex[2].Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);

	vertex[2].TexCoord = XMFLOAT2(0.0f, 1.0f);

	vertex[3].Position = XMFLOAT3(4.0f, 0.0f, 0.0f);  /* 右下。 */

	vertex[3].Normal = XMFLOAT3(0.0f, 0.0f, -1.0f);

	vertex[3].Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);

	vertex[3].TexCoord = XMFLOAT2(1.0f, 1.0f);

	/* 頂点バッファの作成。 */

	D3D11_BUFFER_DESC bd{};

	bd.Usage = D3D11_USAGE_DEFAULT;

	bd.ByteWidth = sizeof(VERTEX_3D) * 4;

	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;

	bd.CPUAccessFlags = 0;

	D3D11_SUBRESOURCE_DATA sd{};

	sd.pSysMem = vertex;

	Renderer::GetDevice()->CreateBuffer(&bd, &sd, &m_VertexBuffer);

	/* シェーダーの作成。 */

	Renderer::CreateVertexShader(&m_VertexShader, &m_VertexLayout, "shader\\unlitTextureVS.cso");

	Renderer::CreatePixelShader(&m_PixelShader, "shader\\unlitTexturePS.cso");

	/* テクスチャの作成。 */

	TexMetadata metadata{};

	ScratchImage image{};

	LoadFromWICFile(L"texture\\grass.png", WIC_FLAGS_NONE, &metadata, image);

	CreateShaderResourceView(Renderer::GetDevice(), image.GetImages(), image.GetImageCount(), metadata, &m_Texture);

}

void Grass::Uninit()

{

	m_VertexBuffer->Release();

	m_VertexLayout->Release();

	m_VertexShader->Release();
	m_PixelShader->Release();
	m_Texture->Release(); /* テクスチャの解放。 */

}

void Grass::Update()

{

}

void Grass::Draw()
{
	Renderer::GetDeviceContext()->IASetInputLayout(m_VertexLayout);
	Renderer::GetDeviceContext()->VSSetShader(m_VertexShader, NULL, 0);
	Renderer::GetDeviceContext()->PSSetShader(m_PixelShader, NULL, 0);
	Renderer::GetDeviceContext()->PSSetShaderResources(0, 1, &m_Texture); /* テクスチャをピクセルシェーダーへバインド。 */

	/* ビルボード行列の作成（Y軸ビルボード）。 */

	Camera* camera = Manager::GetGameObject<Camera>();
	XMMATRIX view = camera->GetViewMatrix();
	XMMATRIX invView = XMMatrixInverse(NULL, view);

	/* Y軸周りの回転のみを残し、X軸・Z軸方向の回転成分をクリアする。 */

	invView.r[0].m128_f32[1] = 0.0f; /* RightベクトルのY成分を0に。 */
	invView.r[1].m128_f32[0] = 0.0f; /* Upベクトルを (0, 1, 0) に固定。 */
	invView.r[1].m128_f32[1] = 1.0f;
	invView.r[1].m128_f32[2] = 0.0f;
	invView.r[2].m128_f32[1] = 0.0f; /* ForwardベクトルのY成分を0に。 */

	/* ベクトルの長さを1に再正規化。 */
	XMVECTOR right = XMVector3Normalize(invView.r[0]);
	XMVECTOR up = XMVector3Normalize(invView.r[1]);
	XMVECTOR forward = XMVector3Normalize(invView.r[2]);

	invView.r[0] = right;
	invView.r[1] = up;   // ← 追加
	invView.r[2] = forward;

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
}
