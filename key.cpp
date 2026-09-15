#include "main.h"
#include "key.h"
#include "renderer.h"
#include "modelRenderer.h"
#include "player.h"
#include "manager.h"
#include "gameStats.h" 

namespace
{
	const float kPickupRange = 1.2f;
}

void Key::Init()
{
	GameStats::NotifyKeySpawned();

	m_Scale = { 0.35f, 0.35f, 0.35f };
	m_Position.y = 1.0f;

	ModelRenderer* modelRenderer = AddComponent<ModelRenderer>();
	modelRenderer->Load("model\\Key_Pickup.obj");

	Renderer::CreateVertexShader(&m_VertexShader, &m_VertexLayout, "shader\\unlitTextureVS.cso");
	Renderer::CreatePixelShader(&m_PixelShader, "shader\\unlitTexturePS.cso");
}

void Key::Uninit()
{
	if (m_VertexLayout) { m_VertexLayout->Release(); m_VertexLayout = nullptr; }
	if (m_VertexShader) { m_VertexShader->Release(); m_VertexShader = nullptr; }
	if (m_PixelShader) { m_PixelShader->Release(); m_PixelShader = nullptr; }
}

void Key::Update()
{
	const float dt = 1.0f / 60.0f;
	m_Rotation.y += 2.0f * dt; 

	Player* player = Manager::GetGameObject<Player>();
	if (player)
	{
		Vector3 direction = player->GetPosition() - m_Position;
		direction.y = 0.0f;
		if (direction.length() < kPickupRange)
		{
			player->AddKey(m_KeyId);
			player->PlayPickupSE();

			GameStats::NotifyKeyCollected();

			SetDestroy(true);
		}
	}

	GameObject::Update();
}

void Key::Draw()
{
	Renderer::GetDeviceContext()->IASetInputLayout(m_VertexLayout);

	Renderer::GetDeviceContext()->VSSetShader(m_VertexShader, NULL, 0);
	Renderer::GetDeviceContext()->PSSetShader(m_PixelShader, NULL, 0);

	XMMATRIX world, scale, rot, trans;
	scale = XMMatrixScaling(m_Scale.x, m_Scale.y, m_Scale.z);
	rot = XMMatrixRotationRollPitchYaw(m_Rotation.x, m_Rotation.y, m_Rotation.z);
	trans = XMMatrixTranslation(m_Position.x, m_Position.y - m_Scale.y, m_Position.z);
	world = scale * rot * trans;

	Renderer::SetWorldMatrix(world);

	GameObject::Draw();
}
