#pragma once
#include "vector3.h"
#include "component.h"
#include <d3d11.h>
#include <list>
#include <type_traits>
#include <DirectXMath.h>

using namespace DirectX;

class GameObject
{
protected: // サブクラスが扱えるように protected
	bool m_Destroy = false;
	int m_Layer = 1;

	float m_CameraZ;

	Vector3 m_Position{ 0,0,0 };
	Vector3 m_Rotation{ 0,0,0 };
	Vector3 m_Scale{ 1,1,1 };

	std::list<Component*> m_Components;

	GameObject* m_Parent = nullptr;

	bool m_Active = true;

public:

	int GetLayer() { return m_Layer; }

	float GetCameraZ() const { return m_CameraZ; }
	void CalCameraZ(Vector3 CameraPosition, 
		Vector3 CameraForward)
	{
		Vector3 direction = m_Position - CameraPosition;
		m_CameraZ = Vector3::dot(direction, CameraForward);
	}

	void SetPosition(const Vector3& position) { m_Position = position; }
	Vector3 GetPosition() { return m_Position; }

	void SetRotation(const Vector3& rotation) { m_Rotation = rotation; }
	Vector3 GetRotation() const { return m_Rotation; }

	void SetScale(const Vector3& scale) { m_Scale = scale; }
	Vector3 GetScale() const { return m_Scale; }

	void SetDestroy(bool destroy) { m_Destroy = true; }

	// ライフサイクル
	virtual void Init(){}

	virtual void Uninit()
	{
		for (Component* component : m_Components)
		{
			if (component)
			{
				component->Uninit();
				delete component;
			}
		}
		m_Components.clear();
	}

	virtual void Update()
	{
		for (Component* component : m_Components)
		{
			if (component) component->Update();
		}
	}

	virtual void Draw()
	{
		for (Component* component : m_Components)
		{
			if (component) component->Draw();
		}
	}

	template <typename T>
	T* AddComponent()
	{
		static_assert(std::is_base_of<Component, T>::value, "T must inherit from Component");
		T* component = new T(this);
		m_Components.push_back(component);
		component->Init();
		return component;
	}

	Vector3 GetForward()
	{
		// rot をローカル変数として定義
		XMMATRIX rot = XMMatrixRotationRollPitchYaw(m_Rotation.x, m_Rotation.y, m_Rotation.z); // 回転量
		// XMVECTOR から Vector3 へ変換
		Vector3 forward;
		XMStoreFloat3((XMFLOAT3*)&forward, rot.r[2]); // rot の前方ベクトルは r[2] に格納されている 
		return forward;
	}
	Vector3 GetRight()
	{
		// rot をローカル変数として定義
		XMMATRIX rot = XMMatrixRotationRollPitchYaw(m_Rotation.x, m_Rotation.y, m_Rotation.z); // 回転量
		// XMVECTOR から Vector3 へ変換
		Vector3 forward;
		XMStoreFloat3((XMFLOAT3*)&forward, rot.r[0]); // rot の前方ベクトルは r[2] に格納されている 
		return forward;
	}
	bool Destroy()
	{
		if(m_Destroy)
		{
			return true;
		}
		else
		{
			return false;
		}
	}
	bool IsDestroy() const
	{
		return m_Destroy;
	}
	// アクティブ状態の設定と取得
	void SetActive(bool active)
	{
		m_Active = active;
	}

	bool GetActive() const
	{
		return m_Active;
	}
};
