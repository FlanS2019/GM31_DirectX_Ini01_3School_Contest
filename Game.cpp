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
#include "horror.h" 
#include "pauseMenu.h" 
#include "settingsScreen.h" 
#include "gameStats.h" 
#include "menuSound.h" 
#include "gameStartText.h" 
#include "loadingScreen.h"
#include <list>


void Game::Init()
{
	GameStats::Reset();

	Manager::AddGameObject<LoadingScreen>();
}

void Game::Uninit()
{
	Input::SetMouseCaptureEnabled(false); 
}

void Game::Update()
{
	if (!Manager::IsPaused())
	{
		GameStats::Tick();
	}

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