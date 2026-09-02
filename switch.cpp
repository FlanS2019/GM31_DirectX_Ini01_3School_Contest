#include "main.h"
#include "switch.h"
#include "door.h"
#include "renderer.h"
#include "modelRenderer.h"
#include "player.h"
#include "manager.h"
#include "Input.h"

namespace
{
	const float kInteractRange = 2.0f;
}

void Switch::Init()
{
	// A low pedestal rather than a full wall block -- shorter than a Key so
	// the two gimmick types don't look interchangeable at a glance.
	m_Scale = { 0.5f, 0.4f, 0.5f };

	ModelRenderer* modelRenderer = AddComponent<ModelRenderer>();
	modelRenderer->Load("model\\switch.obj");

	Renderer::CreateVertexShader(&m_VertexShader, &m_VertexLayout, "shader\\unlitTextureVS.cso");
	Renderer::CreatePixelShader(&m_PixelShader, "shader\\unlitTexturePS.cso");
}

void Switch::Uninit()
{
	if (m_VertexLayout) { m_VertexLayout->Release(); m_VertexLayout = nullptr; }
	if (m_VertexShader) { m_VertexShader->Release(); m_VertexShader = nullptr; }
	if (m_PixelShader) { m_PixelShader->Release(); m_PixelShader = nullptr; }
}

void Switch::Update()
{
	if (!m_Used && m_TargetDoor)
	{
		Player* player = Manager::GetGameObject<Player>();
		if (player)
		{
			Vector3 direction = player->GetPosition() - m_Position;
			if (direction.length() < kInteractRange && Input::GetKeyTrigger('E'))
			{
				m_TargetDoor->Open();
				m_Used = true;
			}
		}
	}

	GameObject::Update();
}

void Switch::Draw()
{
	Renderer::GetDeviceContext()->IASetInputLayout(m_VertexLayout);

	Renderer::GetDeviceContext()->VSSetShader(m_VertexShader, NULL, 0);
	Renderer::GetDeviceContext()->PSSetShader(m_PixelShader, NULL, 0);

	// switch.obj shares box.obj's local convention -- see key.cpp's Draw().
	XMMATRIX world, scale, rot, trans;
	scale = XMMatrixScaling(m_Scale.x, m_Scale.y, m_Scale.z);
	rot = XMMatrixRotationRollPitchYaw(m_Rotation.x, m_Rotation.y, m_Rotation.z);
	trans = XMMatrixTranslation(m_Position.x, m_Position.y - m_Scale.y, m_Position.z);
	world = scale * rot * trans;

	Renderer::SetWorldMatrix(world);

	GameObject::Draw();
}
