//particle.cpp

#include "particle.h"
#include "main.h"
#include "renderer.h"
#include "manager.h"
#include "camera.h"
#include <winerror.h>

void Particle::Init()

{
	m_Layer = 4; /* 描画順を木よりも前にするため、レイヤーを2に設定。 */
	VERTEX_3D vertex[4];

	/* 頂点のY座標に高さを設定し、板ポリゴンとしての面積を持たせます。 */

	vertex[0].Position = XMFLOAT3(-0.5f, 0.5f, 0.0f); /* 左上。 */
	vertex[0].Normal = XMFLOAT3(0.0f, 0.0f, -1.0f);
	vertex[0].Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	vertex[0].TexCoord = XMFLOAT2(0.0f, 0.0f);

	vertex[1].Position = XMFLOAT3(0.5f, 0.5f, 0.0f);  /* 右上。 */
	vertex[1].Normal = XMFLOAT3(0.0f, 0.0f, -1.0f);
	vertex[1].Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	vertex[1].TexCoord = XMFLOAT2(1.0f, 0.0f);

	vertex[2].Position = XMFLOAT3(-0.5f, -0.5f, 0.0f); /* 左下。 */
	vertex[2].Normal = XMFLOAT3(0.0f, 0.0f, -1.0f);
	vertex[2].Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	vertex[2].TexCoord = XMFLOAT2(0.0f, 1.0f);

	vertex[3].Position = XMFLOAT3(0.5f, -0.5f, 0.0f);  /* 右下。 */
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
	LoadFromWICFile(L"texture\\particle.png", WIC_FLAGS_NONE, &metadata, image);
	CreateShaderResourceView(Renderer::GetDevice(), image.GetImages(), image.GetImageCount(), metadata, &m_Texture);
}

void Particle::Uninit()
{
	m_VertexBuffer->Release();
	m_VertexLayout->Release();
	m_VertexShader->Release();
	m_PixelShader->Release();
	m_Texture->Release(); /* テクスチャの解放。 */
}

void Particle::Update()
{
	float dt = 1.0f / 60.0f;
	int count = 30;	

	for (int i = 0; i < MAX_PARTICLES; i++)
	{
		if (m_Particle[i].Enable == false)
		{
			m_Particle[i].Enable = true; /* 寿命が尽きたら無効化。 */
			m_Particle[i].Life = 60; /* 寿命の初期化。 */
			m_Particle[i].Position = m_Position;

			m_Particle[i].Velocity.x = ((float)rand() / RAND_MAX - 0.5f) * 10.0f;
			m_Particle[i].Velocity.y = ((float)rand() / RAND_MAX) * 12.0f;
			m_Particle[i].Velocity.z = ((float)rand() / RAND_MAX - 0.5f) * 10.0f;

			count--;
			if (count <= 0)
			break;
		}
	}

	//パーティクル更新
	Vector3 gravity = { 0.0f, -9.8f, 0.0f };
	for (int i = 0; i < MAX_PARTICLES; i++)
	{
		if(m_Particle[i].Enable == true)
		{
			m_Particle[i].Velocity += gravity * dt; /* 重力の影響を加える。 */
			m_Particle[i].Position += m_Particle[i].Velocity * dt; /* 位置の更新。 */
			/* 寿命が尽きたら無効化。 */
			 if (m_Particle[i].Life <= 0)
			 {
				 m_Particle[i].Enable = false;
			 }
			 else
			 {
				 m_Particle[i].Life--; /* 寿命の減少。 */
			 }
		}
	}
}

void Particle::Draw()

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
	XMVECTOR forward = XMVector3Normalize(invView.r[2]);

	invView.r[0] = right;
	invView.r[2] = forward;

	/* 平行移動成分をクリア。 */
	invView.r[3].m128_f32[0] = 0.0f;
	invView.r[3].m128_f32[1] = 0.0f;
	invView.r[3].m128_f32[2] = 0.0f;

	//XMMATRIX world, scale, trans;

	//scale = XMMatrixScaling(m_Scale.x, m_Scale.y, m_Scale.z); /* 拡大縮小。 */
	//trans = XMMatrixTranslation(m_Position.x, m_Position.y, m_Position.z); /* 平行移動。 */
	//world = scale * invView * trans;

	//Renderer::SetWorldMatrix(world);

	MATERIAL material{};
	material.Diffuse = { 1.0f, 1.0f, 1.0f, 1.0f };
	material.TextureEnable = TRUE; /* テクスチャ有効化。 */

	Renderer::SetMaterial(material);
	//Renderer::GetDeviceContext()->Draw(4, 0);

	UINT stride = sizeof(VERTEX_3D);
	UINT offset = 0;
	Renderer::GetDeviceContext()->IASetVertexBuffers(0, 1, &m_VertexBuffer, &stride, &offset);
	Renderer::GetDeviceContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
	
	for (int i = 0; i < MAX_PARTICLES; i++)
	{
		if (m_Particle[i].Enable == true)
		{
			XMMATRIX world, scale, trans;

			scale = XMMatrixScaling(m_Scale.x, m_Scale.y, m_Scale.z); /* 拡大縮小。 */
			trans = XMMatrixTranslation(
				m_Particle[i].Position.x,
				m_Particle[i].Position.y,
				m_Particle[i].Position.z);			world = scale * invView * trans;

			Renderer::SetWorldMatrix(world);
			Renderer::GetDeviceContext()->Draw(4, 0);
		}
	}
}
