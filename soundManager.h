#pragma once

#include <vector>

class Audio;

class SoundManager
{
private:
	struct Entry
	{
		Audio* audio;
		float baseVolume;
	};

	static std::vector<Entry> s_BgmEntries;
	static std::vector<Entry> s_SeEntries;

	static float s_PauseAttenuation;

public:
	static void RegisterBgm(Audio* audio, float baseVolume = 1.0f);
	static void RegisterSe(Audio* audio, float baseVolume = 1.0f);

	static void Unregister(Audio* audio);
	static void ApplyVolumes();

	static void SetPauseAttenuation(float attenuation);
};
