#include "main.h"
#include "player.h"
#include "renderer.h"
#include "modelRenderer.h"
#include "Input.h"
#include "camera.h"
#include "manager.h"
#include "bullet.h"
#include <cmath>
#include <algorithm>
#include "tree.h"
#include "box.h"
#include "audio.h"
#include "shadow.h"

void Player::Init()
{
	m_Layer = 2;

	m_Position = { 0, 0, 0 };// 初期位置を設定

	ModelRenderer* modelRenderer = AddComponent<ModelRenderer>();
	modelRenderer->Load("model\\player.obj");

	Renderer::CreateVertexShader(&m_VertexShader, &m_VertexLayout, "shader\\unlitTextureVS.cso");
	Renderer::CreatePixelShader(&m_PixelShader, "shader\\unlitTexturePS.cso");

	m_JumpSE = AddComponent<Audio>();
	m_JumpSE->Load("audio\\wan.mp3");

	m_Shadow = Manager::AddGameObject<Shadow>();
	m_Shadow->SetScale({ 5.0f, 5.0f, 5.0f });
}

void Player::Uninit()
{
	if (m_VertexLayout) { m_VertexLayout->Release(); m_VertexLayout = nullptr; }
	if (m_VertexShader) { m_VertexShader->Release(); m_VertexShader = nullptr; }
	if (m_PixelShader) { m_PixelShader->Release(); m_PixelShader = nullptr; }
	if (m_JumpSE) { m_JumpSE->Uninit(); }
}

void Player::Update()
{
	// フレーム固定の簡易 dt（実際は実時間差を使うのが望ましい）
	float dt = 1.0f / 60.0f;
	// パラメータ
	const float accel = 1.0f;    // 加速度 (units/s^2)
	const float maxSpeed = 5.0f;  // 最大速度 (units/s)
	const float friction = 15.0f; // 減速 (units/s^2)
	const float gravity = 60.0f;   // 重力 (units/s^2)
	const float jumpImpulse = 25.0f; // ジャンプ初速 (units/s)

	// --- Sprint (Shift) ---
	const float sprintMultiplier = Input::GetKeyPress(VK_SHIFT) ? 2.0f : 1.0f;
	const float currentAccel = accel * sprintMultiplier;
	const float currentMaxSpeed = maxSpeed * sprintMultiplier;

	// --- カメラ基準の移動方向を計算 ---
	Camera* camera = Manager::GetGameObject<Camera>();
	Vector3 camForward(0.0f, 0.0f, 1.0f);
	Vector3 camRight(1.0f, 0.0f, 0.0f);

	if (camera != nullptr)
	{
		Vector3 camPos = camera->GetPosition();
		Vector3 camTarget = camera->GetTarget(); // ← Camera側にGetTarget()が必要

		camForward = camTarget - camPos;
		camForward.y = 0.0f; // 水平成分だけ使う
		float camForwardLen = camForward.length();
		if (camForwardLen > 0.0001f)
		{
			camForward.x /= camForwardLen;
			camForward.z /= camForwardLen;
		}
		camRight = Vector3(camForward.z, 0.0f, -camForward.x); // Forwardを90度回転
	}

	// 移動入力（カメラ基準）
	bool moving = false;
	float inputX = 0.0f;
	float inputZ = 0.0f;

	if (Input::GetKeyPress('D')) { inputX += 1.0f; moving = true; } // 右
	if (Input::GetKeyPress('A')) { inputX -= 1.0f; moving = true; } // 左
	if (Input::GetKeyPress('W')) { inputZ += 1.0f; moving = true; } // 前
	if (Input::GetKeyPress('S')) { inputZ -= 1.0f; moving = true; } // 後ろ

	// カメラ基準の移動方向ベクトルを計算
	Vector3 moveDir = camForward * inputZ + camRight * inputX;
	float moveLen = moveDir.length();
	if (moveLen > 0.0001f)
	{
		moveDir.x /= moveLen;
		moveDir.z /= moveLen;
	}

	// 地面判定（小さな許容誤差を使用）
	const float groundEpsilon = 0.001f;
	bool grounded = (m_Position.y <= groundEpsilon);
	bool oldGround = m_Grounded;
	m_Grounded = false;

	// 地面上でのジャンプトリガーはここで判定
	if (grounded)
	{
		m_Position.y = 0.0f;
		if (Input::GetKeyTrigger(VK_SPACE))
		{
			m_Velocity.y = jumpImpulse;

			m_Scale.x = 1.0f;
			m_Scale.y = 1.5f; // ジャンプしたときに一瞬伸びる
			m_Scale.z = 1.0f;
		}
		else
		{
			m_Scale.x = 1.0f;
			m_Scale.y = 1.0f;
			m_Scale.z = 1.0f;
			m_JumpSE->Play(); // ← ジャンプ音再生
		}

		if (!oldGround && m_Grounded)
		{
			m_Scale.x = 1.0f;
			m_Scale.y = 0.5f; // ジャンプから着地したときに一瞬潰れる
			m_Scale.z = 1.0f;
		}
	}

	// 重力を適用
	m_Velocity.y -= gravity * dt;

	// --- 摩擦（地面上の水平速度にのみ適用） ---
	Vector3 horizontalVel(m_Velocity.x, 0.0f, m_Velocity.z);
	float hSpeed = std::sqrt(horizontalVel.x * horizontalVel.x + horizontalVel.z * horizontalVel.z);

	if (grounded)
	{
		if (moving)
		{
			m_Velocity.x += moveDir.x * 15.0f * dt;
			m_Velocity.z += moveDir.z * 15.0f * dt;

			float currentSpeed = std::sqrt(m_Velocity.x * m_Velocity.x + m_Velocity.z * m_Velocity.z);
			if (currentSpeed > maxSpeed * sprintMultiplier)
			{
				float inv = maxSpeed / currentSpeed;
				m_Velocity.x *= inv;
				m_Velocity.z *= inv;
			}

			// --- 目標の向きへ滑らかに回転させる ---
			float targetYaw = atan2f(moveDir.x, moveDir.z);
			float diff = targetYaw - m_Rotation.y;

			// 角度差を-πからπの範囲に正規化（最短経路で回転させるため）
			while (diff > XM_PI)  diff -= XM_2PI;
			while (diff < -XM_PI) diff += XM_2PI;

			const float rotateSpeed = 10.0f; // 大きいほど速く向きを変える。好みで調整
			float maxStep = rotateSpeed * dt;
			if (diff > maxStep)      diff = maxStep;
			else if (diff < -maxStep) diff = -maxStep;

			m_Rotation.y += diff;
		}
		else
		{
			// 移動入力がない場合は摩擦で減速する
			if (hSpeed > 0.0f)
			{
				float decel = friction * dt;
				if (decel >= hSpeed)
				{
					m_Velocity.x = 0.0f;
					m_Velocity.z = 0.0f;
				}
				else
				{
					float inv = 1.0f / hSpeed;
					m_Velocity.x -= m_Velocity.x * inv * decel;
					m_Velocity.z -= m_Velocity.z * inv * decel;
				}
			}
		}
	}

	// 速度を位置に反映
	m_Position.x += m_Velocity.x * dt;
	m_Position.y += m_Velocity.y * dt;
	m_Position.z += m_Velocity.z * dt;

	// 地面衝突判定（位置反映後に行う）
	if (m_Position.y < 0.0f)
	{
		m_Position.y = 0.0f;
		if (m_Velocity.y < 0.0f) m_Velocity.y = 0.0f;
		m_Grounded = true;
	}

	// 木の当たり判定
	auto trees = Manager::GetGameObjects<Tree>();
	for (auto tree : trees)
	{
		Vector3 direction = tree->GetPosition() - m_Position;
		float length = direction.length();
		if (length < 1.0f)
		{
			Vector3 pushDir = direction * (1.0f - length);
			m_Position -= pushDir;
		}
	}

	// ボックスとの衝突判定
	auto boxes = Manager::GetGameObjects<Box>();
	for (auto box : boxes)
	{
		Vector3 boxPosition = box->GetPosition();
		Vector3 boxScale = box->GetScale();
		if (boxPosition.x - boxScale.x < m_Position.x && m_Position.x < boxPosition.x + boxScale.x &&
			boxPosition.y - boxScale.y < m_Position.y && m_Position.y < boxPosition.y + boxScale.y &&
			boxPosition.z - boxScale.z < m_Position.z && m_Position.z < boxPosition.z + boxScale.z)
		{
			if (boxPosition.y + boxScale.y - m_Position.y < m_Position.y - (boxPosition.y - boxScale.y))
			{
				m_Position.y = boxPosition.y + boxScale.y;
				if (m_Velocity.y < 0.0f) m_Velocity.y = 0.0f;
				m_Grounded = true;
			}
			else
			{
				if (m_Position.x < boxPosition.x)
					m_Position.x = boxPosition.x - boxScale.x;
				else
					m_Position.x = boxPosition.x + boxScale.x;
			}
		}
	}

	if (Input::GetKeyTrigger('F')) // 弾発射
	{
		OutputDebugStringA("Bullet Create\n");
		Bullet* bullet = Manager::AddGameObject<Bullet>();
		bullet->SetPosition(m_Position);
		bullet->SetVelocity(GetForward() * 25.0f);
	}

	if (m_Grounded)
	{
		m_MoveAnimetion += m_Velocity.length() * dt;
		m_Scale.y += sinf(m_MoveAnimetion * 3.0f) * 0.05f;
	}

	Vector3 shadowPos = m_Position;
	shadowPos.y = 0.05f;
	m_Shadow->SetPosition(shadowPos);

	GameObject::Update();
}


void Player::Draw()
{
	Renderer::GetDeviceContext()->IASetInputLayout(m_VertexLayout);

	Renderer::GetDeviceContext()->VSSetShader(m_VertexShader, NULL, 0);
	Renderer::GetDeviceContext()->PSSetShader(m_PixelShader, NULL, 0);

	XMMATRIX world, scale, rot, trans;
	scale = XMMatrixScaling(m_Scale.x, m_Scale.y, m_Scale.z);//拡大率
	rot = XMMatrixRotationRollPitchYaw(m_Rotation.x, m_Rotation.y + XM_PI, m_Rotation.z);//回転量
	trans = XMMatrixTranslation(m_Position.x, m_Position.y, m_Position.z);//平行移動量
	world = scale * rot * trans;

	Renderer::SetWorldMatrix(world);

	GameObject::Draw();
}