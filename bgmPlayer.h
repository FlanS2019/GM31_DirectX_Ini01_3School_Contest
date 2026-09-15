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
		SoundManager::RegisterBgm(m_Bgm); 
	}
	void Uninit() override
	{
		if (m_Bgm)
		{
			SoundManager::Unregister(m_Bgm); 
			m_Bgm->Uninit(); 
		}
	}
	void Update() override {}
	void Draw() override {}
};