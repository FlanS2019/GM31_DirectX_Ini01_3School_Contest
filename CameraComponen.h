//#pragma once
//#include "component.h"
//#include "vector3.h"
//#include <DirectXMath.h>
//using namespace DirectX;
//
//class CameraComponent : public Component
//{
//private:
//	Vector3 m_Target{ 0,0,0 };
//	Vector3 m_Up{ 0,1,0 };
//
//public:
//	CameraComponent(GameObject* Object) : Component(Object) {}
//	virtual ~CameraComponent() {}
//
//	void Init() override {}
//	void Uninit() override {}
//
//	void Update() override
//	{
//		// Transform を GameObject の位置として利用して View 行列を更新
//		Vector3 pos = m_GameObject->GetPosition();
//		XMVECTOR Eye = XMVectorSet(pos.x, pos.y, pos.z, 0.0f);
//		XMVECTOR At = XMVectorSet(m_Target.x, m_Target.y, m_Target.z, 0.0f);
//		XMVECTOR Up = XMVectorSet(m_Up.x, m_Up.y, m_Up.z, 0.0f);
//
//		XMMATRIX view = XMMatrixLookAtLH(Eye, At, Up);
//
//		// Renderer::SetViewMatrix を使える前提 (renderer.h に宣言あり)
//		extern void Renderer__SetViewMatrix_ForCamera(XMMATRIX); // 宣言の衝突を避けるため外部ラッパー名にしている場合は置き換えてください。
//		// 既存の Renderer::SetViewMatrix を直接使う場合は下行のコメントを外してください:
//		// Renderer::SetViewMatrix(view);
//
//		// 呼び出し（プロジェクト側の Renderer 実装に合わせて調整してください）
//		Renderer::SetViewMatrix(view);
//	}
//
//	void Draw() override {}
//
//	void SetTarget(const Vector3& target) { m_Target = target; }
//	Vector3 GetTarget() const { return m_Target; }
//};