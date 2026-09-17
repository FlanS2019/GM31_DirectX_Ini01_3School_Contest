#include "main.h"
#include "itemBox.h"
#include "renderer.h"
#include "modelRenderer.h"
#include "player.h"
#include "manager.h"
#include "interact.h"
#include "horror.h" // STEP15: TriggerJumpScare() on open

void ItemBox::Init()
{
	// STEP37: 「見た目を変えてほしい」-- 素のbox.obj(壁と同じ質感で埋没して
	// 見えていた)から、既にCrate(crate.cpp)で使っている木箱モデルへ変更。
	// Crate_2x.mtlも一緒に同梱されているので新規アセット無しで済む。
	// crate.h曰く実寸スケール(横1.2～1.3m、高さ0.42m程度)で作られている
	// とのことなので、Crateと同じくm_Scale=1、pivotシフト無しで使う
	// (Draw()側も合わせてある)。
	m_Scale = { 1.0f, 1.0f, 1.0f };

	ModelRenderer* modelRenderer = AddComponent<ModelRenderer>();
	modelRenderer->Load("model\\Crate_2x.obj"); // STEP37: was model\box.obj (placeholder)

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
	return AllItemsCollected() ? "E 開ける" : "E 調べる";
}

void ItemBox::Interact()
{
	if (m_Opened) return;

	if (!AllItemsCollected())
	{
		Interact::ShowWarning("まだ何か足りないようだ...");
		return;
	}

	Player* player = Manager::GetGameObject<Player>();
	if (player)
	{
		// STEP14: hotbar "使ったら消える" request -- the 3 supply items are
		// spent now that they've been deposited, so drop them from the
		// inventory mask the same way Door::Interact() drops a used key.
		for (int id : m_RequiredItemIds)
		{
			if (id >= 0) player->RemoveKey(id);
		}

		if (m_FinalKeyId >= 0)
			player->AddKey(m_FinalKeyId);
	}

	m_Opened = true;

	// STEP37: 開いたときに何も画面に出ないとの指摘 -- 上の警告と同じ
	// Interact::ShowWarning()を流用して、成功時にもメッセージを出す。
	Interact::ShowWarning("組み上げた鍵を手に入れた！", 2.5f);

	// STEP15: scripted scare the moment the box opens -- a startling SE +
	// a quick screen flash (see horror.h/.cpp). Doesn't touch item/key
	// handling above at all, just an extra one-line hook.
	Horror* horror = Manager::GetGameObject<Horror>();
	if (horror) horror->TriggerJumpScare();
}

void ItemBox::Draw()
{
	Renderer::GetDeviceContext()->IASetInputLayout(m_VertexLayout);

	Renderer::GetDeviceContext()->VSSetShader(m_VertexShader, NULL, 0);
	Renderer::GetDeviceContext()->PSSetShader(m_PixelShader, NULL, 0);

	// STEP37: Crate_2x.obj shares Crate::Draw()'s convention -- its own
	// origin already sits at/near the base, so m_Position is used
	// directly with no pivot shift (see crate.cpp's Draw()).
	XMMATRIX world, scale, rot, trans;
	scale = XMMatrixScaling(m_Scale.x, m_Scale.y, m_Scale.z);
	rot = XMMatrixRotationRollPitchYaw(m_Rotation.x, m_Rotation.y, m_Rotation.z);
	trans = XMMatrixTranslation(m_Position.x, m_Position.y, m_Position.z);
	world = scale * rot * trans;

	Renderer::SetWorldMatrix(world);

	GameObject::Draw();
}
