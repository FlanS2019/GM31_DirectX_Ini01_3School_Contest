#include "main.h"
#include "horror.h"
#include "audio.h"
#include "hud.h"
#include "light.h"
#include "manager.h"
#include "soundManager.h"
#include "gameSettings.h"
#include <cstdlib>

namespace
{
	const float kVignetteStrength = 0.55f;     // 常時の縁の暗さ(懐中電灯ON時)
	const float kVignetteStrengthDark = 0.8f;  // 懐中電灯OFF時はさらに濃く

	// STEP21: 両方のアンビエントSE共通 -- 幅のあるランダム間隔にして、メトロノームに聞こえないように(light.cppのフリッカー間隔と同じ考え方)。
	const float kAmbientMinInterval = 22.0f;
	const float kAmbientMaxInterval = 50.0f;

	float RandomRange(float lo, float hi)
	{
		return lo + (hi - lo) * (float(rand()) / float(RAND_MAX));
	}

	// STEP24: 設定画面の「ホラー演出の強さ」(0=弱,1=中,2=強)を反映するための
	// 小さな倍率ヘルパー2つ。GameSettings/SettingsScreen側は0..2の整数しか
	// 知らなくてよく、実際の演出パラメータへの写像はここに閉じ込める。
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
	SoundManager::RegisterSe(m_Heartbeat, 1.0f); // STEP24

	m_ScareSting = AddComponent<Audio>();
	m_ScareSting->Load("audio\\SE\\zyosei1-warai1.mp3");
	SoundManager::RegisterSe(m_ScareSting, 1.0f); // STEP24

	// STEP21: 不定期アンビエントSE、2本 -- 笑い声はm_ScareStingと同じmp3だが、別のAudioコンポーネントに
	// してある(上のクラスコメント参照)。
	m_AmbientLaugh = AddComponent<Audio>();
	m_AmbientLaugh->Load("audio\\SE\\zyosei1-warai1.mp3");
	SoundManager::RegisterSe(m_AmbientLaugh, 1.0f); // STEP24
	m_AmbientLaughTimer = RandomRange(kAmbientMinInterval, kAmbientMaxInterval) * HorrorIntervalScale(); // STEP24: 「ホラー演出の強さ」を反映

	m_DragSE = AddComponent<Audio>();
	m_DragSE->Load("audio\\SE\\sei_ge_hikizuru01.mp3");
	SoundManager::RegisterSe(m_DragSE, 1.0f); // STEP24
	m_DragTimer = RandomRange(kAmbientMinInterval, kAmbientMaxInterval) * HorrorIntervalScale(); // STEP24
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

	// STEP21: 不定期アンビエントSE -- カウントダウンが0以下になったフレームで1回再生して、
	// 次のランダム間隔を引き直す。TriggerJumpScare()とは完全に独立。
	m_AmbientLaughTimer -= 1.0f / 60.0f;
	if (m_AmbientLaughTimer <= 0.0f)
	{
		if (m_AmbientLaugh) m_AmbientLaugh->Play(false);
		m_AmbientLaughTimer = RandomRange(kAmbientMinInterval, kAmbientMaxInterval) * HorrorIntervalScale(); // STEP24
	}

	m_DragTimer -= 1.0f / 60.0f;
	if (m_DragTimer <= 0.0f)
	{
		if (m_DragSE) m_DragSE->Play(false);
		m_DragTimer = RandomRange(kAmbientMinInterval, kAmbientMaxInterval) * HorrorIntervalScale(); // STEP24
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

	// STEP24: 「ホラー演出の強さ」設定 -- 1.0を超えないようクランプする
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
}
