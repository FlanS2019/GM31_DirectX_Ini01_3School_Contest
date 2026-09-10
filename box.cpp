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
	//
	// STEP10: draws m_Scale/m_Position directly again, with NO separate
	// visual-only thinning -- the previous pass thinned the mesh here in
	// Draw() while leaving collision (m_Scale) at the old full-cell size,
	// which meant Player.cpp's collision box no longer matched what was
	// drawn (walls looked thin but still blocked movement like the old
	// thick shape). The thinning now happens once, in Map.cpp, directly on
	// the wall's actual SetPosition()/SetScale() at placement time -- so
	// whatever this draws IS the collision box, always in sync.
	XMMATRIX world, scale, rot, trans;
	scale = XMMatrixScaling(m_Scale.x, m_Scale.y, m_Scale.z);
	rot = XMMatrixRotationRollPitchYaw(m_Rotation.x, m_Rotation.y, m_Rotation.z);
	trans = XMMatrixTranslation(m_Position.x, m_Position.y - m_Scale.y, m_Position.z);
	world = scale * rot * trans;

	Renderer::SetWorldMatrix(world);

	// box.obj's UV is baked for one small (native half-extent 1.0) cube and
	// doesn't tile -- fine for a single cell's wall/ceiling box, but
	// Map.cpp's SpawnMergedWalls/SpawnMergedCeiling can make this box much
	// bigger than one cell now, which was stretching that same texture
	// patch across the whole span instead of repeating it (the "違和感"
	// after walls got merged). Re-tile relative to a plain, unthinned,
	// unmerged cell's half-extent (CELL_SIZE/2 = 2.0): a single cell (half
	// 2.0) still gets tiling 1 (today's look, unchanged), while a run
	// merged 3 cells wide (half 6.0) now repeats the texture 3 times along
	// its length instead of smearing it 3x. SetWorldMatrix() just reset
	// this to (1,1) for every object, so this override only affects Box.
	Renderer::SetUVTiling(m_Scale.x / 2.0f, m_Scale.z / 2.0f);

	GameObject::Draw();
}
