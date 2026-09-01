//manager.h
#pragma once
#include <list>
#include <vector>

class GameObject;
class m_Scene;
class Scene;


class Manager
{
private:
	static std::list<GameObject*> g_GameObject;

	static Scene* m_Scene;
	static Scene* m_NextScene;
	static float m_ChangeTime;

public:
	static void Init();
	static void Uninit();
	static void Update();
	static void Draw();

	template<typename T>
	static void ChangeScene(float Time = 0.0f)
	{
		if(m_NextScene == nullptr)
		{
			m_ChangeTime = Time;
			m_NextScene = new T();
		}
	}

	template<typename T>
	static T* AddGameObject()
	{
		T* gameObject = new T();
		gameObject->Init();
		g_GameObject.push_back(gameObject);
		return gameObject;
	}


	// 単一の最初の一致を返す
	template<typename T>
	static T* GetGameObject()
	{
		for (GameObject* gameObject : g_GameObject)
		{
			T* find = dynamic_cast<T*>(gameObject);
			if (find != nullptr)
				return find;
		}
		return nullptr;
	}

	// 複数一致を返す（名前を変更）
	template<typename T>
	static std::vector<T*> GetGameObjects()
	{
		std::vector<T*> gameObjects;
		for (GameObject* gameObject : g_GameObject)
		{
			T* find = dynamic_cast<T*>(gameObject);
			if (find != nullptr)
				gameObjects.push_back(find);
		}
		return gameObjects;
	}
};