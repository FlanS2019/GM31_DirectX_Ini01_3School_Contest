#include "main.h"
#include "switch.h"
#include "door.h"
#include "renderer.h"
#include "modelRenderer.h"
#include "player.h"
#include "manager.h"
#include "Input.h"
#include "interact.h" // STEP18: switch open feedback text -- see Switch::Interact()

void Switch::Init()
{
	m_Scale = { 0.5f, 0.4f, 0.5f };

	ModelRenderer* modelRenderer = AddComponent<ModelRenderer>();
	modelRenderer->Load("model\\Switch_Lever.obj"); // STEP16: レバー型に変更 -- 経緯はswitch.h参照

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
	GameObject::Update();
}

void Switch::Interact()
{
	if (m_Used || !m_TargetDoor) return;

	m_TargetDoor->Open();
	m_Used = true;

	// STEP18: user asked for on-screen feedback when the switch fires, same
	// mechanism Door/ItemBox already use for their own messages.
	Interact::ShowWarning("ガチャン！何かが開いたようだ…");
}

void Switch::Draw()
{
	Renderer::GetDeviceContext()->IASetInputLayout(m_VertexLayout);

	Renderer::GetDeviceContext()->VSSetShader(m_VertexShader, NULL, 0);
	Renderer::GetDeviceContext()->PSSetShader(m_PixelShader, NULL, 0);

	// Switch_Lever.obj / key.obj / box.obj はどれも同じローカル規約(X/Z
	// 中心、Yは0が根元)でモデリングしてあるので、この行列計算は共通の
	// まま変更不要 -- Switch_Lever.obj自体のコメント参照。
	XMMATRIX world, scale, rot, trans;
	scale = XMMatrixScaling(m_Scale.x, m_Scale.y, m_Scale.z);
	rot = XMMatrixRotationRollPitchYaw(m_Rotation.x, m_Rotation.y, m_Rotation.z);
	trans = XMMatrixTranslation(m_Position.x, m_Position.y - m_Scale.y, m_Position.z);
	world = scale * rot * trans;

	Renderer::SetWorldMatrix(world);

	GameObject::Draw();
}
