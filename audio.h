#pragma once

#include <xaudio2.h>
#include "component.h"


class Audio : public Component
{
private:
	static IXAudio2* m_Xaudio;
	static IXAudio2MasteringVoice* m_MasteringVoice;

	IXAudio2SourceVoice* m_SourceVoice{};
	BYTE* m_SoundData{};

	int						m_Length{};
	int						m_PlayLength{};

	bool LoadMp3(const char* FileName, WAVEFORMATEX& outWfx);

public:
	static void InitMaster();
	static void UninitMaster();

	using Component::Component;

	void Uninit();

	void Load(const char* FileName);
	void Play(bool Loop = false);

	void Stop();

	void SetVolume(float volume);

};