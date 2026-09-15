#include "main.h"
#include "menuCamera.h"

namespace
{
	// STEP29: Map.cppのマス目→ワールド座標の換算式
	// ( x=(col-COLS/2+0.5)*CELL_SIZE, z=(row-ROWS/2+0.5)*CELL_SIZE,
	//   COLS=12, ROWS=11, CELL_SIZE=4.0f ) から、通路になっている
	// (row=4, col=5)のマスを暫定の設置場所とした。実機で見て壁に
	// めり込む/変な角度になっている場合は、この値だけ調整すればOK。
	const float kEyeHeight = 1.6f;
	const float kSwayPeriod = 14.0f;    // 首振り1往復にかかる秒数
	const float kSwayAmplitude = 0.25f; // 首振りの振れ幅(ラジアン)
	const float kBasePitch = -0.05f;    // 気持ち下向き

	// camera.cppのCamera::Update()にある「yaw/pitch→前方ベクトル」の
	// 変換式と全く同じもの(見た目の向きの基準をゲーム中カメラと揃える)。
	Vector3 ForwardFromAngles(float yaw, float pitch)
	{
		Vector3 forward;
		forward.x = cosf(pitch) * sinf(yaw);
		forward.y = sinf(pitch);
		forward.z = cosf(pitch) * cosf(yaw);
		forward.normalize();
		return forward;
	}
}

void MenuCamera::Init()
{
	m_TimeAccum = 0.0f;

	m_Position = Vector3(-2.0f, kEyeHeight, -4.0f);
	m_Yaw = 0.0f;
	m_Pitch = kBasePitch;

	m_Target = m_Position + ForwardFromAngles(m_Yaw, m_Pitch);
}

void MenuCamera::Update()
{
	const float dt = 1.0f / 60.0f;
	m_TimeAccum += dt;

	// Player/マウスは見ない。左右にゆっくり首を振るだけの演出。
	m_Yaw = sinf(m_TimeAccum * (XM_2PI / kSwayPeriod)) * kSwayAmplitude;
	m_Pitch = kBasePitch;

	m_Target = m_Position + ForwardFromAngles(m_Yaw, m_Pitch);
}
