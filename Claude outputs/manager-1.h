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

	// STEP37: シーン遷移の黒フェード。m_ChangeTime(既存、切り替えまでの
	// 残り時間)を「フェードアウトの残り時間」として流用し、切り替え直後
	// からm_ChangeTotalTime秒かけて逆にフェードインする側を新設。
	static float m_ChangeTotalTime; // このm_NextSceneが要求したTime(ChangeScene<T>(Time)の値)
	static float m_FadeInTimer;     // 切り替え直後からの残りフェードイン時間
	static float m_FadeInDuration;  // ↑の合計時間(切り替え時にm_ChangeTotalTimeをスナップショット)

	// STEP24: 追加仕様書12項のポーズ機構。PauseMenuがSetPaused()を呼び、
	// Update()側がこれを見てゲームプレイ系オブジェクトのUpdate()だけを
	// 止める(gameObject.hのUpdatesWhilePaused()参照)。
	static bool m_Paused;

public:
	static void Init();
	static void Uninit();
	static void Update();
	static void Draw();

	static void SetPaused(bool paused) { m_Paused = paused; }
	static bool IsPaused() { return m_Paused; }

	// STEP37: 現在の暗転オーバーレイの不透明度(0=何も無い、1=真っ黒)。
	// 各シーンの「Hud::Begin()～End()を自分で開閉している」描画関数
	// (titleMenu.cpp/howToPlayMenu.cpp/resultMenu.cpp/interact.cpp)が、
	// Hud::End()の直前でHud::DrawFullScreenTint(0,0,0, GetFadeAlpha())を
	// 呼んで最前面に重ねる。
	static float GetFadeAlpha();

	// STEP37: 既定値を0.0f→0.4fに変更 -- 「シーン遷移時に黒の暗転フェードを
	// 入れてほしい」との指定で、明示的にTimeを渡していない既存の呼び出し
	// (title.cpp/result.cpp/titleMenu.cpp等)も含め、全てのシーン遷移で
	// フェードがかかるようにする。
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