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
#include "gameStats.h" // STEP25
#include "menuSound.h" // STEP27
#include "gameStartText.h" // STEP32
#include "loadingScreen.h" // STEP40
#include <list>


void Game::Init()
{
	// STEP25: リザルト画面用の「待つまでの時間」「鍵を取った数」を
	// リセット。LoadingScreenが完了する前にはMap::Init()(Key生成)も
	// まだ呼ばれていないので、必ずその前に呼んでおく(gameStats.h参照)。
	GameStats::Reset();

	// STEP40: 以前はここでCamera～GameStartTextの13個を一気に
	// Manager::AddGameObject<T>()(=呼んだ瞬間にT::Init()を同期実行する -- manager.h参照)
	// していたため、ローディング中の進捗表示が一切なかった。その13行と
	// Input::SetMouseCaptureEnabled(true)はすべてloadingScreen.cppの
	// LoadingScreen::Init()にそのまま移し(順番も同じ)、1フレームで1個ずつ
	// 実行しながら進捗(%)と現在のステップ名を画面に出せるようにした。
	Manager::AddGameObject<LoadingScreen>();
}

void Game::Uninit()
{
	Input::SetMouseCaptureEnabled(false); // STEP36
}

void Game::Update()
{
	// STEP25: 一時停止中は「生還までの時間」を進めたくないので、Manager::
	// IsPaused()を見てから呼ぶ(Manager::Update()自体はScene::Update()を
	// ポーズ非依存で毎フレーム呼んでしまうため、ここでガードしている)。
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