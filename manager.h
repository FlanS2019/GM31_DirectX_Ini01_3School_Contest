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

	static float m_ChangeTotalTime; // このm_NextSceneが要求したTime(ChangeScene<T>(Time)の値)
	static float m_FadeInTimer;     // 切り替え直後からの残りフェードイン時間
	static float m_FadeInDuration;  // ↑の合計時間(切り替え時にm_ChangeTotalTimeをスナップショット)

	static bool m_Paused;

public:
	static void Init();
	static void Uninit();
	static void Update();
	static void Draw();

	static void SetPaused(bool paused) { m_Paused = paused; }
	static bool IsPaused() { return m_Paused; }

	static float GetFadeAlpha();

	template<typename T>
	static void ChangeScene(float Time = 0.4f)
	{
		if(m_NextScene == nullptr)
		{
			m_ChangeTime = Time;
			m_ChangeTotalTime = Time;
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