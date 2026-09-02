#include "main.h"
#include "key.h"
#include "renderer.h"
#include "modelRenderer.h"
#include "player.h"
#include "manager.h"

namespace
{
	const float kPickupRange = 1.2f;
}

void Key::Init()
{
	// Small and off the ground so it doesn't read as another wall block;
	// Map.cpp can still override both after AddGameObject<Key>().
	m_Scale = { 0.35f, 0.35f, 0.35f };
	m_Position.y = 1.0f;

	ModelRenderer* modelRenderer = AddComponent<ModelRenderer>();
	modelRenderer->Load("model\\key.obj");

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
	m_Rotation.y += 2.0f * dt; // slow spin so it reads as "pick me up", not a prop

	Player* player = Manager::GetGameObject<Player>();
	if (player)
	{
		Vector3 direction = player->GetPosition() - m_Position;
		if (direction.length() < kPickupRange)
		{
			player->AddKey(m_KeyId);
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

	// key.obj shares box.obj's local convention (X/Z centered, Y from 0 at
	// the base to 2 at the top) so GetPosition() reads as this object's
	// center the same way it does everywhere else -- see box.cpp's Draw().
	XMMATRIX world, scale, rot, trans;
	scale = XMMatrixScaling(m_Scale.x, m_Scale.y, m_Scale.z);
	rot = XMMatrixRotationRollPitchYaw(m_Rotation.x, m_Rotation.y, m_Rotation.z);
	trans = XMMatrixTranslation(m_Position.x, m_Position.y - m_Scale.y, m_Position.z);
	world = scale * rot * trans;

	Renderer::SetWorldMatrix(world);

	GameObject::Draw();
}
