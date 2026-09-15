#include "main.h"
#include "menuSound.h"
#include "audio.h"
#include "soundManager.h"

void MenuSound::Init()
{
	// STEP27: カーソル移動音は決定音より控えめな音量にしてある
	// (baseVolumeの違い -- SoundManager::RegisterSe()参照)。
	m_MoveSE = AddComponent<Audio>();
	m_MoveSE->Load("audio\\SE\\Horror_Accent06-2(Short).mp3");
	SoundManager::RegisterSe(m_MoveSE, 0.7f);

	m_ConfirmSE = AddComponent<Audio>();
	m_ConfirmSE->Load("audio\\SE\\Horror_Accent01-1(High).mp3");
	SoundManager::RegisterSe(m_ConfirmSE, 1.0f);
}

void MenuSound::Uninit()
{
	if (m_MoveSE) { SoundManager::Unregister(m_MoveSE); m_MoveSE->Uninit(); }
	if (m_ConfirmSE) { SoundManager::Unregister(m_ConfirmSE); m_ConfirmSE->Uninit(); }
}

void MenuSound::PlayMove()
{
	if (m_MoveSE) m_MoveSE->Play(false);
}

void MenuSound::PlayConfirm()
{
	if (m_ConfirmSE) m_ConfirmSE->Play(false);
}
