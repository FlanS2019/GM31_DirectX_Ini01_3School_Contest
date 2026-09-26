#pragma once

#include "gameObject.h"

class Box : public GameObject
{
private:
	Vector3 m_Velocity{ 0,0,0 };

	ID3D11InputLayout* m_VertexLayout;
	ID3D11VertexShader* m_VertexShader;
	ID3D11PixelShader* m_PixelShader;

	bool m_Blocking = true;

public:
	void Init()override;
	void Uninit()override;
	void Update()override;
	void Draw()override;

	virtual bool IsBlocking() const { return m_Blocking; }

	// STEP51: walls/ceiling/door-frames -- never cull. These are exactly the
	// large, structural pieces a generic fixed-radius cull sphere can't
	// safely approximate (see gameObject.h IsCullable()).
	bool IsCullable() const override { return false; }

	void SetBlocking(bool blocking) { m_Blocking = blocking; }

	// STEP02(stage2)-detail: lets a subclass swap the model file Init()
	// loads (see undergroundWall.h) without touching Init()/Uninit()/Draw()
	// or any of the collision plumbing -- Manager::GetGameObjects<Box>()
	// finds subclass instances via dynamic_cast, so a subclass still blocks
	// the player exactly like a plain Box.
	virtual const char* GetModelPath() const { return "model\\box.obj"; }
};
