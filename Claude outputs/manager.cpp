//manager.cpp
#include "main.h"
#include "manager.h"
#include "renderer.h"
#include "Input.h"
#include "camera.h"
#include "gameObject.h"
#include "title.h"
#include "result.h"
#include "Game.h"
#include "gameSettings.h"

std::list<GameObject*> Manager::g_GameObject;//リストを使用する場合は、配列ではなくリストを宣言する必要があります。
Scene* Manager::m_Scene = nullptr;
Scene* Manager::m_NextScene = nullptr;
float Manager::m_ChangeTime = 0.0f;
bool Manager::m_Paused = false;

void Manager::Init()
{
	// STEP24: アプリ起動時に一度だけ -- settings.iniの読み込みを試みる。
	// まだLight/Audioは何も存在しない段階なので、値を保持するだけでよい
	// (各システムへの反映は、それぞれが生成されるタイミングで自分から
	// GameSettingsを読みに行く -- light.cpp Light::Init()等参照)。
	GameSettings::Init();

	Renderer::Init();
	Input::Init();
	ChangeScene<Title>();
	ChangeScene<result>();
}

void Manager::Uninit()
{
	// STEP24: 追加仕様書13項「設定値はアプリ終了時にファイルへ保存」。
	GameSettings::Save();

	if(m_Scene != nullptr)
	{
		m_Scene->Uninit();
		delete m_Scene;
	}

	for(GameObject* gameObject : g_GameObject)
	{
		gameObject->Uninit();
		delete gameObject;
	}
	g_GameObject.clear();
	Renderer::Uninit();
	Input::Uninit();
}

void Manager::Update()
{
	float dt = 1.0f / 60.0f;

	Input::Update();

	if (m_Scene != nullptr)
	{
		m_Scene->Update();
	}

	// アクティブなオブジェクトのみ更新
	// STEP24: ポーズ中(m_Paused)はUpdatesWhilePaused()がtrueのオブジェクト
	// (PauseMenu/SettingsScreen自身)だけを動かし、それ以外(Player/Camera/
	// Horrorのアンビエントタイマー等)は完全に止める -- 追加仕様書19項
	// 「ポーズ中はゲームロジックが進行しないよう、Update呼び出しを確実に
	// 止める」に対応。
	for (GameObject* gameObject : g_GameObject)
	{
		if (gameObject->GetActive() && (!m_Paused || gameObject->UpdatesWhilePaused()))
		{
			gameObject->Update();
		}
	}

	// Destroy対象削除
	for (auto it = g_GameObject.begin(); it != g_GameObject.end(); )
	{
		if ((*it)->IsDestroy())
		{
			(*it)->Uninit();
			delete (*it);
			it = g_GameObject.erase(it);
		}
		else
		{
			++it;
		}
	}

	if (m_NextScene != nullptr)
	{
		m_ChangeTime -= dt;

		if (m_ChangeTime < 0.0f)
		{
			if (m_Scene != nullptr)
			{
				m_Scene->Uninit();
				delete m_Scene;
			}

			for (GameObject* gameObject : g_GameObject)
			{
				gameObject->Uninit();
				delete gameObject;
			}

			g_GameObject.clear();

			// STEP24: シーンが変わった直後にポーズ状態を持ち越さない安全策
			// (通常はPauseMenu::Update()の「タイトルへ戻る」自身が
			// SetPaused(false)してからChangeSceneするが、念のための保険)。
			m_Paused = false;

			m_Scene = m_NextScene;
			m_Scene->Init();

			m_NextScene = nullptr;
		}
	}
}

void Manager::Draw()
{
	Renderer::Begin();

	Camera* camera = GetGameObject<Camera>();

	if (camera)
	{
		camera->Draw();

		Vector3 forward = camera->GetForward();
		Vector3 position = camera->GetPosition();

		for (GameObject* gameObject : g_GameObject)
		{
			gameObject->CalCameraZ(position, forward);
		}

		// Zソート
		g_GameObject.sort([](GameObject* a, GameObject* b)
			{
				return a->GetCameraZ() > b->GetCameraZ();
			});
	}

	for (int layer = 0; layer <= 10; layer++)
	{
		for (GameObject* gameObject : g_GameObject)
		{
			if (gameObject->GetLayer() == layer &&
				gameObject->GetActive())
			{
				gameObject->Draw();
			}
		}
	}
	Renderer::End();
}