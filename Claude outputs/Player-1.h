#pragma once
#include "gameObject.h"
class Shadow;
class Audio;

class Player : public GameObject
{
private:
	Vector3 m_Velocity{ 0,0,0 };
	ID3D11InputLayout* m_VertexLayout = nullptr;
	ID3D11VertexShader* m_VertexShader = nullptr;
	ID3D11PixelShader* m_PixelShader = nullptr;
	bool m_Grounded = true;
	float m_MoveAnimetion = 0.0f;
	float m_CameraZ = 0.0f;
	class Transform* m_Transform = nullptr;
	class Audio* m_JumpSE = nullptr;

	// STEP21: looping footstep SE -- only while actually moving on the
	// ground (see Update()). m_WalkPlaying tracks whether the loop is
	// currently started so Update() only calls Play(true)/Stop() on the
	// frame the state actually changes, not every frame.
	class Audio* m_WalkSE = nullptr;
	bool m_WalkPlaying = false;

	// STEP21: shared one-shot pickup SE -- Key/Item play it via
	// PlayPickupSE() below instead of owning their own Audio component,
	// because both destroy themselves (SetDestroy(true)) the same frame
	// they'd start playing it, which would cut an Audio component living
	// on THEM off almost immediately. Player never gets destroyed, so the
	// sound always finishes.
	class Audio* m_PickupSE = nullptr;

	class Shadow* m_Shadow = nullptr;
public:
	void Init()override;
	void Uninit()override;
	void Update()override;
	void Draw()override;

	void AddKey(int id) { if (id >= 0 && id < 32) m_KeyMask |= (1u << id); }
	bool HasKey(int id) const { return id >= 0 && id < 32 && (m_KeyMask & (1u << id)) != 0; }

	void RemoveKey(int id) { if (id >= 0 && id < 32) m_KeyMask &= ~(1u << id); }

	// STEP21: called by Key::Update()/Item::Interact() on a successful pickup.
	void PlayPickupSE();

private:
	unsigned int m_KeyMask = 0;
};
