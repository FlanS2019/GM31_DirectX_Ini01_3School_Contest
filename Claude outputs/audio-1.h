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

	// STEP15: explicit stop (no restart) for a looping voice -- e.g. the
	// heartbeat loop in horror.h/.cpp needs to actually stop, not restart
	// a one-shot playthrough the way calling Play(false) would.
	void Stop();

	// STEP22: per-voice volume (1.0 = unchanged, e.g. 0.5 = half). "•à‚­‰¹‚Æ
	// “d‹C‚Ì‰¹‚ª‚¤‚é‚³‚¢" -- some SEs (footsteps, the light-flicker
	// crackle) were too loud relative to everything else, and there was no
	// way to turn any one of them down individually.
	void SetVolume(float volume);

};