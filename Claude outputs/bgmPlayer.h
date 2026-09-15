// bgmPlayer.h
#pragma once
#include "gameObject.h"
#include "audio.h"
#include "soundManager.h"

class BgmPlayer : public GameObject
{
private:
	Audio* m_Bgm = nullptr;
public:
	void Init() override
	{
		m_Bgm = AddComponent<Audio>();
		m_Bgm->Load("audio\\BGM\\abandoned_hospital.mp3");
		SoundManager::RegisterBgm(m_Bgm); // STEP24: 設定画面のBGM音量スライダーを反映
		m_Bgm->Play(true); // ループ再生
	}
	void Uninit() override
	{
		if (m_Bgm)
		{
			SoundManager::Unregister(m_Bgm); // STEP24
			m_Bgm->Uninit();  // ← ここ追加：SourceVoiceをStop/Destroy
		}
	}
	void Update() override {}
	void Draw() override {}
};