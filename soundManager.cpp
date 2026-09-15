#include "main.h"
#include "soundManager.h"
#include "audio.h"
#include "gameSettings.h"
#include <algorithm>

std::vector<SoundManager::Entry> SoundManager::s_BgmEntries;
std::vector<SoundManager::Entry> SoundManager::s_SeEntries;
float SoundManager::s_PauseAttenuation = 1.0f;

void SoundManager::RegisterBgm(Audio* audio, float baseVolume)
{
	if (!audio) return;
	s_BgmEntries.push_back({ audio, baseVolume });
	audio->SetVolume(baseVolume * GameSettings::GetBgmVolume());
}

void SoundManager::RegisterSe(Audio* audio, float baseVolume)
{
	if (!audio) return;
	s_SeEntries.push_back({ audio, baseVolume });
	audio->SetVolume(baseVolume * GameSettings::GetSeVolume() * s_PauseAttenuation);
}

void SoundManager::Unregister(Audio* audio)
{
	if (!audio) return;

	auto pred = [audio](const Entry& entry) { return entry.audio == audio; };
	s_BgmEntries.erase(std::remove_if(s_BgmEntries.begin(), s_BgmEntries.end(), pred), s_BgmEntries.end());
	s_SeEntries.erase(std::remove_if(s_SeEntries.begin(), s_SeEntries.end(), pred), s_SeEntries.end());
}

void SoundManager::ApplyVolumes()
{
	float bgmVolume = GameSettings::GetBgmVolume();
	for (auto& entry : s_BgmEntries)
	{
		if (entry.audio) entry.audio->SetVolume(entry.baseVolume * bgmVolume);
	}

	float seVolume = GameSettings::GetSeVolume() * s_PauseAttenuation;
	for (auto& entry : s_SeEntries)
	{
		if (entry.audio) entry.audio->SetVolume(entry.baseVolume * seVolume);
	}
}

void SoundManager::SetPauseAttenuation(float attenuation)
{
	s_PauseAttenuation = attenuation;
	ApplyVolumes();
}
