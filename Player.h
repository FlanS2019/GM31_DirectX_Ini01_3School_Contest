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
	// コンポーネント参照（Init で AddComponent して保持）
	class Transform* m_Transform = nullptr;
	class Audio* m_JumpSE = nullptr; // ジャンプ音
	class Shadow* m_Shadow = nullptr; // シャドウオブジェクトへの参照
public:
	void Init()override;
	void Uninit()override;
	void Update()override;
	void Draw()override;
};