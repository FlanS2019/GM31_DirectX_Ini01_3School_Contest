#pragma once

#include "gameObject.h"

class Audio;

class Horror : public GameObject
{
private:
	Audio* m_Heartbeat = nullptr;
	bool m_HeartbeatPlaying = false;

	Audio* m_ScareSting = nullptr;
	float m_FlashTimer = 0.0f;
	const float m_FlashDuration = 0.35f;

	Audio* m_AmbientLaugh = nullptr;
	float m_AmbientLaughTimer = 0.0f;

	Audio* m_DragSE = nullptr;
	float m_DragTimer = 0.0f;

	bool m_ScaredRoomC = false;
	bool m_ScaredRoomN = false;
	void CheckProximityScares();

	bool m_GlitchActive = false;
	int m_GlitchPhasesRemaining = 0;
	bool m_GlitchDarkPhase = false;
	float m_GlitchPhaseTimer = 0.0f;
	float m_GlitchTimer = 0.0f;
	void UpdateGlitchEvent();

public:
	void Init() override;
	void Uninit() override;
	void Update() override;
	void Draw() override {} 

	void TriggerJumpScare();

	void DrawScreenEffects();
};
