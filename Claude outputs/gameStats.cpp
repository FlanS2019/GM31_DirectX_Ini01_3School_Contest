#include "gameStats.h"

namespace
{
	int g_ElapsedFrames = 0;
	int g_TotalKeys = 0;
	int g_KeysCollected = 0;
}

void GameStats::Reset()
{
	g_ElapsedFrames = 0;
	g_TotalKeys = 0;
	g_KeysCollected = 0;
}

void GameStats::Tick()
{
	g_ElapsedFrames++;
}

void GameStats::NotifyKeySpawned()
{
	g_TotalKeys++;
}

void GameStats::NotifyKeyCollected()
{
	g_KeysCollected++;
}

float GameStats::GetElapsedSeconds()
{
	// Manager::Update()ë§ÇÃdtå≈íËíl(1.0f / 60.0f)Ç…çáÇÌÇπÇƒÇ†ÇÈÅB
	return g_ElapsedFrames / 60.0f;
}

int GameStats::GetKeysCollected()
{
	return g_KeysCollected;
}

int GameStats::GetTotalKeys()
{
	return g_TotalKeys;
}
