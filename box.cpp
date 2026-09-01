#include "main.h"
#include "Box.h"
#include "renderer.h"
#include "modelRenderer.h"
#include "Input.h"

void Box::Init()
{
	//m_Position = { -5, 1, 0 };

	ModelRenderer* modelRenderer = AddComponent<ModelRenderer>();
	modelRenderer->Load("model\\box.obj");

	Renderer::CreateVertexShader(&m_VertexShader, &m_VertexLayout, "shader\\unlitTextureVS.cso");
	Renderer::CreatePixelShader(&m_PixelShader, "shader\\unlitTexturePS.cso");

}
void Box::Uninit()
{
	if (m_VertexLayout) { m_VertexLayout->Release(); m_VertexLayout = nullptr; }
	if (m_VertexShader) { m_VertexShader->Release(); m_VertexShader = nullptr; }
	if (m_PixelShader) { m_PixelShader->Release(); m_PixelShader = nullptr; }
}
void Box::Update()
{
	//m_Position.x += m_Velocity.x;
	//m_Position.y += m_Velocity.y;
	//m_Position.z += m_Velocity.z;
	//float dt = 1.0f / 60;
	//if (Input::GetKeyPress('D'))//migi
	//{
	//	m_Position.x += 5 * dt;
	//}
	//if (Input::GetKeyPress('A'))//hidari
	//{
	//	m_Position.x -= 5 * dt;
	//}
	//if (Input::GetKeyPress('W'))//mae
	//{
	//	m_Position.z -= 5 * dt;
	//}
	//if (Input::GetKeyPress('S'))//ushiro
	//{
	//	m_Position.z += 5 * dt;
	//}
}
void Box::Draw()
{
	Renderer::GetDeviceContext()->IASetInputLayout(m_VertexLayout);

	Renderer::GetDeviceContext()->VSSetShader(m_VertexShader, NULL, 0);
	Renderer::GetDeviceContext()->PSSetShader(m_PixelShader, NULL, 0);

	// box.obj's own pivot sits at its base (local Y runs 0..2), while X/Z
	// are centered (-1..1). Everywhere that uses this class -- the box/wall
	// collision code, Map.cpp's wall placement -- treats GetPosition() as
	// the box's CENTER and GetScale() as its half-extent on every axis. So
	// shift the draw down by m_Scale.y to line the visible mesh up with
	// that center, instead of changing the position/collision convention.
	XMMATRIX world, scale, rot, trans;
	scale = XMMatrixScaling(m_Scale.x, m_Scale.y, m_Scale.z); // scale factor
	rot = XMMatrixRotationRollPitchYaw(m_Rotation.x, m_Rotation.y, m_Rotation.z);
	trans = XMMatrixTranslation(m_Position.x, m_Position.y - m_Scale.y, m_Position.z);
	world = scale * rot * trans;

	Renderer::SetWorldMatrix(world);

	GameObject::Draw();
}