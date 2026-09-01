//#pragma once
//#include "component.h"
//#include "vector3.h"
//#include "gameObject.h" // <- GameObject のメソッドをヘッダ内の inline 実装から呼ぶために完全定義が必要
//
//class Transform : public Component
//{
//private:
//	Vector3 m_Position{ 0,0,0 };
//	Vector3 m_Rotation{ 0,0,0 };
//	Vector3 m_Scale{ 1,1,1 };
//
//public:
//	Transform(GameObject* Object) : Component(Object) {}
//	virtual ~Transform() {}
//
//	void Init() override {}
//	void Uninit() override {}
//
//	void Update() override
//	{
//		// 毎フレーム GameObject の位置をコンポーネント側の値で同期
//		if (m_GameObject) m_GameObject->SetPosition(m_Position);
//	}
//
//	// アクセサ
//	void SetPosition(const Vector3& pos) { m_Position = pos; if (m_GameObject) m_GameObject->SetPosition(m_Position); }
//	Vector3 GetPosition() const { return m_Position; }
//
//	void SetRotation(const Vector3& rot) { m_Rotation = rot; if (m_GameObject) m_GameObject->SetRotation(m_Rotation); }
//	Vector3 GetRotation() const { return m_Rotation; }
//
//	void SetScale(const Vector3& scale) { m_Scale = scale; if (m_GameObject) m_GameObject->SetScale(m_Scale); }
//	Vector3 GetScale() const { return m_Scale; }
//};