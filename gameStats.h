#pragma once

// STEP25: 既存result.cpp(「背景画像+Enterでタイトルへ」だけの状態)を
// 拡張する要望に対応 -- 「生還までの時間」「見つけた鍵の数」をリザルト
// 画面に出す。
//
// Resultシーンに切り替わる瞬間、Manager::Update()のシーン切り替え処理が
// g_GameObjectを丸ごと破棄する(pauseMenu.h/settingsScreen.h等で説明済みの
// 制約と同じ)ため、Game中のPlayer/Key等のGameObjectにこれらの値を持たせて
// おくとResultからは読めなくなる。そのためGameSettingsと同じ「GameObject
// ではない静的クラス」に持たせ、ゲーム中に逐次記録しておいたものを
// Result::Init()側からそのまま読みに行く、という設計にしてある。
class GameStats
{
public:
	// Game::Init()の先頭(Map/Key生成より前)で呼ぶ。前回プレイの値が
	// 残らないようにするためのリセット。
	static void Reset();

	// Game::Update()から毎フレーム呼ぶ想定。「生還までの時間」に一時停止中の
	// 時間を含めたくないため、呼び出し側でManager::IsPaused()を見てから
	// 呼ぶこと(ポーズ中は呼ばない)。
	static void Tick();

	// Key::Init()から呼ぶ。マップに配置された鍵の総数を数えるだけ。
	static void NotifyKeySpawned();

	// Key::Update()の取得判定成立時に呼ぶ。Door::Interact()で鍵を消費して
	// (Player::RemoveKey())も、この「これまでに見つけた数」は減らない --
	// Player側のビットマスクとは別に持っているのはこのため。
	static void NotifyKeyCollected();

	static float GetElapsedSeconds();
	static int GetKeysCollected();
	static int GetTotalKeys();
};
