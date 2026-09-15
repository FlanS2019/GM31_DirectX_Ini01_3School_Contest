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

	class Audio* m_WalkSE = nullptr;
	bool m_WalkPlaying = false;

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

	void PlayPickupSE();

private:
	unsigned int m_KeyMask = 0;
};
