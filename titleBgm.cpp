#include "main.h"
#include "titleBgm.h"
#include "audio.h"
#include "soundManager.h"

void TitleBgm::Init()
{
	m_Bgm = AddComponent<Audio>();
	m_Bgm->Load("audio\\BGM\\Flutter_BGM.mp3");
	SoundManager::RegisterBgm(m_Bgm, 1.0f); 
	m_Bgm->Play(true); // ƒ‹[ƒvÄ¶
}

void TitleBgm::Uninit()
{
	if (m_Bgm) { SoundManager::Unregister(m_Bgm); m_Bgm->Uninit(); }
}
