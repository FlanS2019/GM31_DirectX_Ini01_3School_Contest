#include "main.h"
#include "titleBgm.h"
#include "audio.h"
#include "soundManager.h"

void TitleBgm::Init()
{
	// STEP28: ファイル名は既存BGM("abandoned_hospital.mp3")と同じ
	// audio\BGMフォルダ・同じ拡張子(.mp3)のはず、という前提で書いてある。
	// 実際のファイル名/拡張子が違う場合はここの1行を差し替えるだけでよい。
	m_Bgm = AddComponent<Audio>();
	m_Bgm->Load("audio\\BGM\\Flutter_BGM.mp3");
	SoundManager::RegisterBgm(m_Bgm, 1.0f); // 設定画面のBGM音量スライダーを反映
	m_Bgm->Play(true); // ループ再生
}

void TitleBgm::Uninit()
{
	if (m_Bgm) { SoundManager::Unregister(m_Bgm); m_Bgm->Uninit(); }
}
