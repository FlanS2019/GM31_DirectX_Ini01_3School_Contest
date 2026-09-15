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
#include "splash.h" 

std::list<GameObject*> Manager::g_GameObject;
Scene* Manager::m_Scene = nullptr;
Scene* Manager::m_NextScene = nullptr;
float Manager::m_ChangeTime = 0.0f;
float Manager::m_ChangeTotalTime = 0.0f; 
float Manager::m_FadeInTimer = 0.0f;     
float Manager::m_FadeInDuration = 0.0f;
bool Manager::m_Paused = false;

void Manager::Init()
{
	GameSettings::Init();

	Renderer::Init();
	Input::Init();
	ChangeScene<Splash>();
	ChangeScene<result>();
}

void Manager::Uninit()
{
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

	for (GameObject* gameObject : g_GameObject)
	{
		if (gameObject->GetActive() && (!m_Paused || gameObject->UpdatesWhilePaused()))
		{
			gameObject->Update();
		}
	}

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

			m_Paused = false;

			m_FadeInDuration = m_ChangeTotalTime;
			m_FadeInTimer = m_ChangeTotalTime;

			m_Scene = m_NextScene;
			m_Scene->Init();

			m_NextScene = nullptr;
		}
	}
	else if (m_FadeInTimer > 0.0f)
	{
		m_FadeInTimer -= dt;
		if (m_FadeInTimer < 0.0f) m_FadeInTimer = 0.0f;
	}
}

float Manager::GetFadeAlpha()
{
	if (m_NextScene != nullptr)
	{
		if (m_ChangeTotalTime <= 0.0001f) return 0.0f; // Time=0指定は「フェード無しの瞬時切り替え」のまま
		float t = 1.0f - (m_ChangeTime / m_ChangeTotalTime);
		if (t < 0.0f) t = 0.0f;
		if (t > 1.0f) t = 1.0f;
		return t;
	}

	if (m_FadeInTimer > 0.0f && m_FadeInDuration > 0.0001f)
	{
		float t = m_FadeInTimer / m_FadeInDuration;
		if (t < 0.0f) t = 0.0f;
		if (t > 1.0f) t = 1.0f;
		return t;
	}

	return 0.0f;
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