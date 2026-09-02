#pragma once
#include "gameObject.h"
class Shadow;

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
	// �R���|�[�l���g�Q�ƁiInit �� AddComponent ���ĕێ��j
	class Transform* m_Transform = nullptr;
	class Audio* m_JumpSE = nullptr; // �W�����v��
	class Shadow* m_Shadow = nullptr; // �V���h�E�I�u�W�F�N�g�ւ̎Q��
public:
	void Init()override;
	void Uninit()override;
	void Update()override;
	void Draw()override;

	// Key inventory: STEP2 doors/gimmicks. One bit per key id (id 0..31 -- a
	// bitmask is overkill-proof for a maze this size and needs no container).
	// Key::Update() calls AddKey() on pickup; Door::Update() calls HasKey()
	// before letting a locked door open.
	void AddKey(int id) { if (id >= 0 && id < 32) m_KeyMask |= (1u << id); }
	bool HasKey(int id) const { return id >= 0 && id < 32 && (m_KeyMask & (1u << id)) != 0; }

private:
	unsigned int m_KeyMask = 0;
};