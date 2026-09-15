#include "main.h"
#include "horror.h"
#include "audio.h"
#include "hud.h"
#include "light.h"
#include "manager.h"

namespace
{
	const float kVignetteStrength = 0.55f;     // 常時の縁の暗さ(懐中電灯ON時)
	const float kVignetteStrengthDark = 0.8f;  // 懐中電灯OFF時はさらに濃く
}

void Horror::Init()
{
	m_Heartbeat = AddComponent<Audio>();
	m_Heartbeat->Load("audio\\SE\\Heartbeat03-3(Slow-Loop).mp3");

	m_ScareSting = AddComponent<Audio>();
	m_ScareSting->Load("audio\\SE\\zyosei1-warai1.mp3");
}

void Horror::Uninit()
{
	if (m_Heartbeat) m_Heartbeat->Uninit();
	if (m_ScareSting) m_ScareSting->Uninit();
}

void Horror::Update()
{
	// 懐中電灯がOFFの間だけ心臓の鼓動ループ -- 「暗闇にひとり」の緊張感。
	// Light::IsFlashlightOn()を覗くだけでlight.cpp側には一切手を入れない。
	Light* light = Manager::GetGameObject<Light>();
	bool wantHeartbeat = light && !light->IsFlashlightOn();

	if (wantHeartbeat && !m_HeartbeatPlaying)
	{
		m_Heartbeat->Play(true);
		m_HeartbeatPlaying = true;
	}
	else if (!wantHeartbeat && m_HeartbeatPlaying)
	{
		m_Heartbeat->Stop(); // STEP15: audio.h/.cppに追加したStop() -- Play(false)だと非ループの再生が始まってしまうので使えない
		m_HeartbeatPlaying = false;
	}

	if (m_FlashTimer > 0.0f)
	{
		m_FlashTimer -= 1.0f / 60.0f;
		if (m_FlashTimer < 0.0f) m_FlashTimer = 0.0f;
	}

	GameObject::Update();
}

void Horror::TriggerJumpScare()
{
	if (m_ScareSting) m_ScareSting->Play(false);
	m_FlashTimer = m_FlashDuration;
}

void Horror::DrawScreenEffects()
{
	Light* light = Manager::GetGameObject<Light>();
	bool flashlightOn = light && light->IsFlashlightOn();

	Hud::DrawVignette(flashlightOn ? kVignetteStrength : kVignetteStrengthDark);

	if (m_FlashTimer > 0.0f)
	{
		// m_FlashDuration -> 0へ線形にフェードアウトするだけの簡単な白フラッシュ。
		float alpha = m_FlashTimer / m_FlashDuration;
		Hud::DrawFullScreenTint(1.0f, 1.0f, 1.0f, alpha * 0.85f);
	}
}
