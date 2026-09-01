#include "main.h"
#include "bullet.h"
#include "renderer.h"
#include "modelRenderer.h"
#include "Input.h"
#include "manager.h"
#include "enemy.h"
#include "explosion.h" // 追加
#include "Score.h" // 追加

void Bullet::Init()
{
	m_Layer = 2;
	// 生成側で位置をセットすることを前提にする（Player から SetPosition される）
	 m_Position = { 0, 0, 0 }; // 削除

	ModelRenderer* modelRenderer = AddComponent<ModelRenderer>();
	modelRenderer->Load("model\\bullet.obj");

	Renderer::CreateVertexShader(&m_VertexShader, &m_VertexLayout, "shader\\unlitTextureVS.cso");
	Renderer::CreatePixelShader(&m_PixelShader, "shader\\unlitTexturePS.cso");
}
void Bullet::Uninit()
{
	if (m_VertexLayout) { m_VertexLayout->Release(); m_VertexLayout = nullptr; }
	if (m_VertexShader) { m_VertexShader->Release(); m_VertexShader = nullptr; }
	if (m_PixelShader) { m_PixelShader->Release(); m_PixelShader = nullptr; }
}
void Bullet::Update()
{
	// フレーム固定 dt（プロジェクト全体と合わせる）
	const float dt = 1.0f / 60.0f;

	// 速度を位置に反映
	m_Position += m_Velocity * dt;
	// 敵との当たり判定（単純な距離判定）
	auto enemies = Manager::GetGameObjects<enemy>();
	for (auto enemy : enemies)
	{
		Vector3 direction = enemy->GetPosition() - m_Position;// 弾と敵の位置の差を計算
		float length = direction.length();
		if (length < 0.5f) // 当たり判定の半径（例: 0.5f）
		{
			// 爆発エフェクトを生成
			Explosion* exp = Manager::AddGameObject<Explosion>();
			if (exp)
			{
				exp->SetPosition(enemy->GetPosition());
			}

			// ---- スコア加算 ----
			auto scores = Manager::GetGameObjects<Score>();
			for (auto score : scores)
			{
				score->AddScore(1);
			}

			enemy->SetDestroy(true); // 敵を Destroy
			SetDestroy(true); // 弾も Destroy
			return; // 当たったらこれ以上処理しない
		}
	}

	m_Lifetime -= dt;
	if(m_Lifetime <= 0.0f)
	{
		SetDestroy(true);
	}

	// 必要なら寿命や画面外判定を追加（ここでは単純化）
}
void Bullet::Draw()
{
	Renderer::GetDeviceContext()->IASetInputLayout(m_VertexLayout);

	Renderer::GetDeviceContext()->VSSetShader(m_VertexShader, NULL, 0);
	Renderer::GetDeviceContext()->PSSetShader(m_PixelShader, NULL, 0);

	XMMATRIX world, scale, rot, trans;
	scale = XMMatrixScaling(m_Scale.x, m_Scale.y, m_Scale.z);//拡大率
	rot = XMMatrixRotationRollPitchYaw(m_Rotation.x, m_Rotation.y, m_Rotation.z);//回転量
	trans = XMMatrixTranslation(m_Position.x, m_Position.y, m_Position.z);//平行移動量
	world = scale * rot * trans;

	Renderer::SetWorldMatrix(world);

	GameObject::Draw();
}