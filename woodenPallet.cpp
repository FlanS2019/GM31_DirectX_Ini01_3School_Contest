#include "main.h"
#include "woodenPallet.h"
#include "renderer.h"
#include "modelRenderer.h"

void WoodenPallet::Init()
{
	// See woodenPallet.h's comment -- native units are already real-world
	// scale, so no scaling is applied.
	m_Scale = { 1.0f, 1.0f, 1.0f };

	ModelRenderer* modelRenderer = AddComponent<ModelRenderer>();
	modelRenderer->Load("model\\WoodenPallet_Broken2.obj");

	Renderer::CreateVertexShader(&m_VertexShader, &m_VertexLayout, "shader\\unlitTextureVS.cso");
	Renderer::CreatePixelShader(&m_PixelShader, "shader\\unlitTexturePS.cso");
}

void WoodenPallet::Uninit()
{
	if (m_VertexLayout) { m_VertexLayout->Release(); m_VertexLayout = nullptr; }
	if (m_VertexShader) { m_VertexShader->Release(); m_VertexShader = nullptr; }
	if (m_PixelShader) { m_PixelShader->Release(); m_PixelShader = nullptr; }
}

void WoodenPallet::Draw()
{
	Renderer::GetDeviceContext()->IASetInputLayout(m_VertexLayout);

	Renderer::GetDeviceContext()->VSSetShader(m_VertexShader, NULL, 0);
	Renderer::GetDeviceContext()->PSSetShader(m_PixelShader, NULL, 0);

	// The source FBX's own pivot is NOT centered on the mesh's X/Z footprint
	// (model-space X spans roughly 2.39..3.59, nowhere near 0) -- tried
	// re-centering it here at one point, but that only shifts the mesh
	// sideways along whichever wall it's leaning on (X is unaffected by the
	// standing pitch rotation, so it only ever feeds into the along-wall
	// component after the yaw rotation, never the toward/away-from-wall
	// component) -- it was never the cause of the pallet floating away from
	// the wall. Reverted: m_Position is used directly, and Map.cpp's
	// SpawnLeaningPallet positions callers relative to this raw pivot (see
	// its comment for where the real toward-wall offset math lives).
	XMMATRIX world, scale, rot, trans;
	scale = XMMatrixScaling(m_Scale.x, m_Scale.y, m_Scale.z);
	rot = XMMatrixRotationRollPitchYaw(m_Rotation.x, m_Rotation.y, m_Rotation.z);
	trans = XMMatrixTranslation(m_Position.x, m_Position.y, m_Position.z);
	world = scale * rot * trans;

	Renderer::SetWorldMatrix(world);

	GameObject::Draw();
}
