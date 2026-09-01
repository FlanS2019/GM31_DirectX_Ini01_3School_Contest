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
#include "Game.h"
#include "title.h"
#include "result.h"
#include "Score.h"
#include "bgmPlayer.h"
#include "shadow.h"
#include <list>


void Game::Init()
{
	GameObject* gameObject = nullptr;

	Manager::AddGameObject<Camera>();
	Manager::AddGameObject<Field>();
	Manager::AddGameObject<Player>();
	Manager::AddGameObject<Tree>()->SetPosition({ -10.0f, 0.0f, 10.0f });
	Manager::AddGameObject<Particle>()->SetPosition({ -2.0f, 1.0f, 2.0f });
	//Manager::AddGameObject<Shadow>();
	Manager::AddGameObject<Score>()->Init();
	Manager::AddGameObject<BgmPlayer>();

	//enemyを10個増やす
	for (int i = 0; i < 1; i++)
	{
		Manager::AddGameObject<enemy>()->SetPosition({ -2.0f + i * 12.0f, 0.0f, 7.0f });
	}

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
	auto enemies = Manager::GetGameObjects<enemy>();
	if(enemies.size() == 0)
	{
		Manager::ChangeScene<result>(2.0f);//敵が全滅したら結果画面に遷移
	}
	// パーティクル停止
	if (Input::GetKeyPress(VK_F2))
	{
		auto particles = Manager::GetGameObjects<Particle>();

		for (auto particle : particles)
		{
			particle->SetActive(false);
		}
	}

	// パーティクル再開
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
