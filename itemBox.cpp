#include "main.h"
#include "itemBox.h"
#include "renderer.h"
#include "modelRenderer.h"
#include "player.h"
#include "manager.h"
#include "interact.h"
#include "horror.h" 

void ItemBox::Init()
{
	m_Scale = { 1.0f, 1.0f, 1.0f };

	ModelRenderer* modelRenderer = AddComponent<ModelRenderer>();
	modelRenderer->Load("model\\Crate_2x.obj"); 

	Renderer::CreateVertexShader(&m_VertexShader, &m_VertexLayout, "shader\\unlitTextureVS.cso");
	Renderer::CreatePixelShader(&m_PixelShader, "shader\\unlitTexturePS.cso");
}

void ItemBox::Uninit()
{
	if (m_VertexLayout) { m_VertexLayout->Release(); m_VertexLayout = nullptr; }
	if (m_VertexShader) { m_VertexShader->Release(); m_VertexShader = nullptr; }
	if (m_PixelShader) { m_PixelShader->Release(); m_PixelShader = nullptr; }
}

void ItemBox::Update()
{
	GameObject::Update();
}

bool ItemBox::AllItemsCollected() const
{
	Player* player = Manager::GetGameObject<Player>();
	if (!player) return false;

	for (int id : m_RequiredItemIds)
	{
		if (id < 0) continue; // unset slot -- ignore
		if (!player->HasKey(id)) return false;
	}
	return true;
}

const char* ItemBox::GetInteractText()
{
	return AllItemsCollected() ? "E äJÇØÇÈ" : "E í≤Ç◊ÇÈ";
}

void ItemBox::Interact()
{
	if (m_Opened) return;

	if (!AllItemsCollected())
	{
		OutputDebugStringA("[ItemBox] not all 3 items collected yet.\n");
		Interact::ShowWarning("Ç‹ÇæâΩÇ©ë´ÇËÇ»Ç¢ÇÊÇ§Çæ...");
		return;
	}

	Player* player = Manager::GetGameObject<Player>();
	if (player)
	{
		for (int id : m_RequiredItemIds)
		{
			if (id >= 0) player->RemoveKey(id);
		}

		if (m_FinalKeyId >= 0)
			player->AddKey(m_FinalKeyId);
	}

	m_Opened = true;
	OutputDebugStringA("[ItemBox] opened -- final key obtained.\n");

	Interact::ShowWarning("ëgÇ›è„Ç∞ÇΩåÆÇéËÇ…ì¸ÇÍÇΩÅI", 2.5f);

	Horror* horror = Manager::GetGameObject<Horror>();
	if (horror) horror->TriggerJumpScare();
}

void ItemBox::Draw()
{
	Renderer::GetDeviceContext()->IASetInputLayout(m_VertexLayout);

	Renderer::GetDeviceContext()->VSSetShader(m_VertexShader, NULL, 0);
	Renderer::GetDeviceContext()->PSSetShader(m_PixelShader, NULL, 0);

	XMMATRIX world, scale, rot, trans;
	scale = XMMatrixScaling(m_Scale.x, m_Scale.y, m_Scale.z);
	rot = XMMatrixRotationRollPitchYaw(m_Rotation.x, m_Rotation.y, m_Rotation.z);
	trans = XMMatrixTranslation(m_Position.x, m_Position.y, m_Position.z);
	world = scale * rot * trans;

	Renderer::SetWorldMatrix(world);

	GameObject::Draw();
}
