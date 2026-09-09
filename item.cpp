#include "main.h"
#include "item.h"
#include "renderer.h"
#include "modelRenderer.h"
#include "player.h"
#include "manager.h"
#include <cstdio>

void Item::Init()
{
	// Small and off the ground, same reasoning as Key -- doesn't read as
	// another wall block.
	m_Scale = { 0.3f, 0.3f, 0.3f };
	m_Position.y = 1.0f;

	ModelRenderer* modelRenderer = AddComponent<ModelRenderer>();
	modelRenderer->Load("model\\key.obj"); // placeholder art -- swap per-item once real models exist

	Renderer::CreateVertexShader(&m_VertexShader, &m_VertexLayout, "shader\\unlitTextureVS.cso");
	Renderer::CreatePixelShader(&m_PixelShader, "shader\\unlitTexturePS.cso");
}

void Item::Uninit()
{
	if (m_VertexLayout) { m_VertexLayout->Release(); m_VertexLayout = nullptr; }
	if (m_VertexShader) { m_VertexShader->Release(); m_VertexShader = nullptr; }
	if (m_PixelShader) { m_PixelShader->Release(); m_PixelShader = nullptr; }
}

void Item::Update()
{
	const float dt = 1.0f / 60.0f;
	m_Rotation.y += 2.0f * dt; // slow spin so it reads as "pick me up", same as Key

	GameObject::Update();
}

const char* Item::GetInteractText()
{
	sprintf_s(m_PromptBuf, "E %sÇèEÇ§", m_DisplayName);
	return m_PromptBuf;
}

void Item::Interact()
{
	if (m_Collected) return;

	Player* player = Manager::GetGameObject<Player>();
	if (player)
	{
		player->AddKey(m_ItemId);
	}

	m_Collected = true;
	SetDestroy(true); // picked up -- remove from the world, same as Key's old auto-pickup did

	char buf[96];
	sprintf_s(buf, "[Item] picked up: %s (id=%d)\n", m_DisplayName, m_ItemId);
	OutputDebugStringA(buf);
}

void Item::Draw()
{
	Renderer::GetDeviceContext()->IASetInputLayout(m_VertexLayout);

	Renderer::GetDeviceContext()->VSSetShader(m_VertexShader, NULL, 0);
	Renderer::GetDeviceContext()->PSSetShader(m_PixelShader, NULL, 0);

	// key.obj shares box.obj's local convention (X/Z centered, Y from 0 at
	// the base) -- see key.cpp's Draw() for the same math.
	XMMATRIX world, scale, rot, trans;
	scale = XMMatrixScaling(m_Scale.x, m_Scale.y, m_Scale.z);
	rot = XMMatrixRotationRollPitchYaw(m_Rotation.x, m_Rotation.y, m_Rotation.z);
	trans = XMMatrixTranslation(m_Position.x, m_Position.y - m_Scale.y, m_Position.z);
	world = scale * rot * trans;

	Renderer::SetWorldMatrix(world);

	GameObject::Draw();
}