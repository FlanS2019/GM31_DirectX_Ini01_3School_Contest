#include "main.h"
#include "item.h"
#include "renderer.h"
#include "modelRenderer.h"
#include "player.h"
#include "manager.h"
#include <cstdio>

namespace
{
	const char* GetModelPathForItemId(int id)
	{
		switch (id)
		{
		case 1: return "model\\Item_Photo.obj";
		case 2: return "model\\Item_Letter.obj";
		case 3: return "model\\Item_MetalPart.obj";
		default: return "model\\key.obj";
		}
	}
}

void Item::Init()
{
	m_Scale = { 0.3f, 0.3f, 0.3f };
	m_Position.y = 1.0f;

	m_ModelRenderer = AddComponent<ModelRenderer>();
	m_ModelRenderer->Load(GetModelPathForItemId(m_ItemId)); // m_ItemIdはまだ既定値(0)のことが多い -- SetItemId()が呼ばれた時点で本来のモデルに差し替わる

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
	m_Rotation.y += 2.0f * dt;

	GameObject::Update();
}

const char* Item::GetInteractText()
{
	sprintf_s(m_PromptBuf, "E %sを拾う", m_DisplayName);
	return m_PromptBuf;
}

void Item::SetItemId(int id)
{
	m_ItemId = id;

	if (m_ModelRenderer)
		m_ModelRenderer->Load(GetModelPathForItemId(id));
}

void Item::Interact()
{
	if (m_Collected) return;

	Player* player = Manager::GetGameObject<Player>();
	if (player)
	{
		player->AddKey(m_ItemId);
		player->PlayPickupSE(); 
	}

	m_Collected = true;
	SetDestroy(true); 

	char buf[96];
	sprintf_s(buf, "[Item] picked up: %s (id=%d)\n", m_DisplayName, m_ItemId);
	OutputDebugStringA(buf);
}

void Item::Draw()
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
