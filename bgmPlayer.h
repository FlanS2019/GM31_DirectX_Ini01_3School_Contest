// bgmPlayer.h
#pragma once
#include "gameObject.h"
#include "audio.h"

class BgmPlayer : public GameObject
{
private:
	Audio* m_Bgm = nullptr;
public:
	void Init() override
	{
		m_Bgm = AddComponent<Audio>();
		m_Bgm->Load("audio\\BGM\\bgm.wav");
		m_Bgm->Play(true); // ÉãÅ[Évçƒê∂
	}
	void Uninit() override
	{
		if (m_Bgm)
		{
			m_Bgm->Uninit();  // Å© Ç±ÇÍÇí«â¡ÅFSourceVoiceÇÃStop/Destroy
		}
	}
	void Update() override {}
	void Draw() override {}
};