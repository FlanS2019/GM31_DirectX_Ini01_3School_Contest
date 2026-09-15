//game.cpp
#include "main.h"
#include "manager.h"
#include "renderer.h"
#include "polygon2d.h"
#include "Input.h"
#include "field.h"
#include "camera.h"
#include "player.h"
#include "gameObject.h"
#include "enemy.h"
#include "bullet.h"
#include "tree.h"
#include "grass.h"
#include "explosion.h"
#include "box.h"
#include "particle.h"
#include "Map.h"
#include "Game.h"
#include "title.h"
#include "result.h"
#include "Score.h"
#include "bgmPlayer.h"
#include "shadow.h"
#include "light.h"
#include "interact.h"
#include "horror.h" // STEP15: ambient heartbeat + vignette + jump scares
#include "pauseMenu.h" // STEP24
#include "settingsScreen.h" // STEP24
#include <list>


void Game::Init()
{
	GameObject* gameObject = nullptr;

	Manager::AddGameObject<Camera>();
	Manager::AddGameObject<Light>(); // STEP4: scene lighting (dim baseline; see light.h)
	Manager::AddGameObject<Field>();
	Manager::AddGameObject<Player>();
	Manager::AddGameObject<Map>(); // ruins blockout (walls); Field above is still the floor
	Manager::AddGameObject<Score>(); // AddGameObject<T>() already calls Init() once -- an extra ->Init() here used to run Score::Init() twice (leaked its old vertex buffer/shaders/texture)
	Manager::AddGameObject<BgmPlayer>();
	Manager::AddGameObject<Interact>(); // STEP6: interact system (see interact.h)

	// STEP24: 追加仕様書12/13項 -- ESCキーでの一時停止と、そこから開く
	// 設定画面。どちらも「開かれるまでは何もしない」オーバーレイなので、
	// ここで一度追加しておけばよい(interact.cppのDrawUI()呼び出し参照)。
	Manager::AddGameObject<PauseMenu>();
	Manager::AddGameObject<SettingsScreen>();
	Manager::AddGameObject<Horror>(); // STEP15: heartbeat/vignette/jump scares (see horror.h)

	//Box* box = Manager::AddGameObject<Box>();
	//box->SetPosition({ 2.0f, 0.0f, 5.0f });
	//box->SetScale({ 2.0f, 2.0f, 2.0f });

	//Manager::AddGameObject<Polygon2D>()->Init(0.0f, 0.0f, 200.0f, 200.0f,L"texture\\jimen.jpg");
	//Manager::AddGameObject<Grass>()->SetPosition({ 5.0f, 0.0f, 3.0f });
	//Manager::AddGameObject<Explosion>()->SetPosition({ 0.0f, 0.0f, 5.0f });
	//Manager::AddGameObject<Bullet>();
}

void Game::Uninit()
{
}

void Game::Update()
{
	// stop particles
	if (Input::GetKeyPress(VK_F2))
	{
		auto particles = Manager::GetGameObjects<Particle>();

		for (auto particle : particles)
		{
			particle->SetActive(false);
		}
	}

	// resume particles
	if (Input::GetKeyPress(VK_F3))
	{
		auto particles = Manager::GetGameObjects<Particle>();

		for (auto particle : particles)
		{
			particle->SetActive(true);
		}
	}
}

void Game::Draw()
{
}