#include "main.h"
#include "horror.h"
#include "audio.h"
#include "hud.h"
#include "light.h"
#include "manager.h"
#include "soundManager.h"
#include "gameSettings.h"
#include "player.h" 
#include <cstdlib>

namespace
{
	const float kVignetteStrength = 0.55f;     // 常時の縁の暗さ(懐中電灯ON時)
	const float kVignetteStrengthDark = 0.8f;  // 懐中電灯OFF時はさらに濃く

	const float kAmbientMinInterval = 22.0f;
	const float kAmbientMaxInterval = 50.0f;

	const float kProximityScareRadius = 4.0f;

	const float kGlitchMinInterval = 35.0f;
	const float kGlitchMaxInterval = 70.0f;
	const float kGlitchPhaseDuration = 0.08f; // 1フェーズ(暗 or 明)の長さ
	const int   kGlitchTotalPhases = 5;       // 暗→明→暗→明→暗 のように交互に切り替わる回数

	float RandomRange(float lo, float hi)
	{
		return lo + (hi - lo) * (float(rand()) / float(RAND_MAX));
	}

	float HorrorIntensityScale() // ヴィネット/フラッシュの強さに掛ける
	{
		switch (GameSettings::GetHorrorIntensity())
		{
		case 0: return 0.7f;  // 弱
		case 2: return 1.35f; // 強
		default: return 1.0f; // 中
		}
	}

	float HorrorIntervalScale() // 不定期アンビエントSEの間隔に掛ける(小さいほど高頻度)
	{
		switch (GameSettings::GetHorrorIntensity())
		{
		case 0: return 1.6f;  // 弱: 間隔を伸ばす(控えめ)
		case 2: return 0.6f;  // 強: 間隔を縮める(頻繁)
		default: return 1.0f; // 中
		}
	}
}

void Horror::Init()
{
	m_Heartbeat = AddComponent<Audio>();
	m_Heartbeat->Load("audio\\SE\\Heartbeat03-3(Slow-Loop).mp3");
	SoundManager::RegisterSe(m_Heartbeat, 1.0f); 

	m_ScareSting = AddComponent<Audio>();
	m_ScareSting->Load("audio\\SE\\zyosei1-warai1.mp3");
	SoundManager::RegisterSe(m_ScareSting, 1.0f); 

	m_AmbientLaugh = AddComponent<Audio>();
	m_AmbientLaugh->Load("audio\\SE\\zyosei1-warai1.mp3");
	SoundManager::RegisterSe(m_AmbientLaugh, 1.0f); // 
	m_AmbientLaughTimer = RandomRange(kAmbientMinInterval, kAmbientMaxInterval) * HorrorIntervalScale(); //: 「ホラー演出の強さ」を反映

	m_DragSE = AddComponent<Audio>();
	m_DragSE->Load("audio\\SE\\sei_ge_hikizuru01.mp3");
	SoundManager::RegisterSe(m_DragSE, 1.0f); 
	m_DragTimer = RandomRange(kAmbientMinInterval, kAmbientMaxInterval) * HorrorIntervalScale(); 

	m_GlitchTimer = RandomRange(kGlitchMinInterval, kGlitchMaxInterval) * HorrorIntervalScale(); 
}

void Horror::Uninit()
{
	if (m_Heartbeat) { SoundManager::Unregister(m_Heartbeat); m_Heartbeat->Uninit(); }
	if (m_ScareSting) { SoundManager::Unregister(m_ScareSting); m_ScareSting->Uninit(); }
	if (m_AmbientLaugh) { SoundManager::Unregister(m_AmbientLaugh); m_AmbientLaugh->Uninit(); }
	if (m_DragSE) { SoundManager::Unregister(m_DragSE); m_DragSE->Uninit(); }
}

void Horror::Update()
{
	Light* light = Manager::GetGameObject<Light>();
	bool wantHeartbeat = light && !light->IsFlashlightOn();

	if (wantHeartbeat && !m_HeartbeatPlaying)
	{
		m_Heartbeat->Play(true);
		m_HeartbeatPlaying = true;
	}
	else if (!wantHeartbeat && m_HeartbeatPlaying)
	{
		m_Heartbeat->Stop(); 
		m_HeartbeatPlaying = false;
	}

	if (m_FlashTimer > 0.0f)
	{
		m_FlashTimer -= 1.0f / 60.0f;
		if (m_FlashTimer < 0.0f) m_FlashTimer = 0.0f;
	}

	m_AmbientLaughTimer -= 1.0f / 60.0f;
	if (m_AmbientLaughTimer <= 0.0f)
	{
		if (m_AmbientLaugh) m_AmbientLaugh->Play(false);
		m_AmbientLaughTimer = RandomRange(kAmbientMinInterval, kAmbientMaxInterval) * HorrorIntervalScale(); 
	}

	m_DragTimer -= 1.0f / 60.0f;
	if (m_DragTimer <= 0.0f)
	{
		if (m_DragSE) m_DragSE->Play(false);
		m_DragTimer = RandomRange(kAmbientMinInterval, kAmbientMaxInterval) * HorrorIntervalScale(); 
	}

	CheckProximityScares(); 
	UpdateGlitchEvent();

	GameObject::Update();
}

void Horror::TriggerJumpScare()
{
	if (m_ScareSting) m_ScareSting->Play(false);
	m_FlashTimer = m_FlashDuration;
}

void Horror::CheckProximityScares()
{
	Player* player = Manager::GetGameObject<Player>();
	if (!player) return;
	Vector3 pos = player->GetPosition();

	if (!m_ScaredRoomC)
	{
		float dx = pos.x - 14.0f;
		float dz = pos.z - (-12.0f);
		if (dx * dx + dz * dz <= kProximityScareRadius * kProximityScareRadius)
		{
			TriggerJumpScare();
			m_ScaredRoomC = true;
		}
	}

	if (!m_ScaredRoomN)
	{
		float dx = pos.x - 14.0f;
		float dz = pos.z - 4.0f;
		if (dx * dx + dz * dz <= kProximityScareRadius * kProximityScareRadius)
		{
			TriggerJumpScare();
			m_ScaredRoomN = true;
		}
	}
}

void Horror::UpdateGlitchEvent()
{
	if (!m_GlitchActive)
	{
		m_GlitchTimer -= 1.0f / 60.0f;
		if (m_GlitchTimer <= 0.0f)
		{
			m_GlitchActive = true;
			m_GlitchDarkPhase = true;
			m_GlitchPhasesRemaining = kGlitchTotalPhases;
			m_GlitchPhaseTimer = kGlitchPhaseDuration;

			Light* light = Manager::GetGameObject<Light>();
			if (light) light->StartFlicker();
		}
		return;
	}

	m_GlitchPhaseTimer -= 1.0f / 60.0f;
	if (m_GlitchPhaseTimer <= 0.0f)
	{
		m_GlitchPhasesRemaining--;
		if (m_GlitchPhasesRemaining <= 0)
		{
			m_GlitchActive = false;
			m_GlitchDarkPhase = false;

			Light* light = Manager::GetGameObject<Light>();
			if (light) light->StopFlicker();

			m_GlitchTimer = RandomRange(kGlitchMinInterval, kGlitchMaxInterval) * HorrorIntervalScale();
		}
		else
		{
			m_GlitchDarkPhase = !m_GlitchDarkPhase;
			m_GlitchPhaseTimer = kGlitchPhaseDuration;
		}
	}
}

void Horror::DrawScreenEffects()
{
	Light* light = Manager::GetGameObject<Light>();
	bool flashlightOn = light && light->IsFlashlightOn();

	// (strengthは不透明度そのものなので、1.0を超えると意味がない)。
	float vignette = (flashlightOn ? kVignetteStrength : kVignetteStrengthDark) * HorrorIntensityScale();
	if (vignette > 1.0f) vignette = 1.0f;
	Hud::DrawVignette(vignette);

	if (m_FlashTimer > 0.0f)
	{
		// m_FlashDuration -> 0へ線形にフェードアウトするだけの簡単な白フラッシュ。
		float alpha = m_FlashTimer / m_FlashDuration;
		float tint = alpha * 0.85f * HorrorIntensityScale();
		if (tint > 1.0f) tint = 1.0f;
		Hud::DrawFullScreenTint(1.0f, 1.0f, 1.0f, tint);
	}

	if (m_GlitchActive && m_GlitchDarkPhase)
	{
		Hud::DrawFullScreenTint(0.0f, 0.0f, 0.0f, 0.92f);
	}
}
