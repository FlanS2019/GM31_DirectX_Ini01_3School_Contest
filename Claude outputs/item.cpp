#include "main.h"
#include "item.h"
#include "renderer.h"
#include "modelRenderer.h"
#include "player.h"
#include "manager.h"
#include <cstdio>

namespace
{
	// STEP: 「拾う3つの物資のモデルを作ってほしい」との要望で追加。
	// Map.cppのSetItemId()呼び出しは1='古い写真'/2='色あせた手紙'/
	// 3='金属部品'で固定(P/R/Mの各caseを参照)なので、そのIDをそのまま
	// モデルパスに変換するだけの単純な対応表。0や未知のIDはInit()が
	// SetItemId()より前に呼ばれた直後の一瞬だけ通る値なので、その間だけ
	// 元のkey.obj(仮モデル)にフォールバックしておく。
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
	// Small and off the ground, same reasoning as Key -- doesn't read as
	// another wall block.
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
	m_Rotation.y += 2.0f * dt; // slow spin so it reads as "pick me up", same as Key

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

	// Init()は生成直後にAddComponent<ModelRenderer>()するだけの箱を
	// 作っているので、ここで本来のIDに応じたモデルに差し替える。
	// ModelRenderer::Load()は同じファイル名なら2回目以降キャッシュを
	// 使うだけなので、呼び出しコスト自体は気にしなくていい。
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
		player->PlayPickupSE(); // STEP21: "ものを取る音" -- shared with Key::Update(), see Player.h's m_PickupSE comment
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

	// key.obj / Item_Photo.obj / Item_Letter.obj / Item_MetalPart.obj は
	// どれも同じローカル規約(X/Z中心、Yは0が根元)でモデリングしてあるので、
	// この行列計算は共通のまま変更不要 -- 各モデルのコメント参照。
	XMMATRIX world, scale, rot, trans;
	scale = XMMatrixScaling(m_Scale.x, m_Scale.y, m_Scale.z);
	rot = XMMatrixRotationRollPitchYaw(m_Rotation.x, m_Rotation.y, m_Rotation.z);
	trans = XMMatrixTranslation(m_Position.x, m_Position.y - m_Scale.y, m_Position.z);
	world = scale * rot * trans;

	Renderer::SetWorldMatrix(world);

	GameObject::Draw();
}
