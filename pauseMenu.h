#pragma once

#include "gameObject.h"

// STEP24: 追加仕様書12項「一時停止(ポーズ)画面」。GameシーンにGame::Init()
// から一度だけ追加され(Horror/Interactと同じパターン)、ゲーム中ずっと
// 生き続ける -- ESCキーが押されるまでは何もしない。
//
// PAUSE/SETTINGSはTITLE/HOW TO PLAY/GAME/RESULTのような独立シーンではなく、
// GAMEシーンの上に重ねて表示する軽量な「オーバーレイ」として扱う(11項)。
// そのためシーン遷移(Manager::ChangeScene)は使わず、Manager::SetPaused()で
// ゲームプレイ系オブジェクトのUpdate()だけを止める(gameObject.hの
// UpdatesWhilePaused()参照)。
//
// 描画はHud::Begin()/End()の対がフレームに1組だけという制約があるため、
// Horror::DrawScreenEffects()と同じパターンで、Interact::Draw()の中にある
// 既存のHud::Begin()/End()から明示的にDrawUI()を呼んでもらう(Draw()自体は
// 空のまま)。
class PauseMenu : public GameObject
{
public:
	enum class State
	{
		Closed,       // 非表示、ゲーム通常進行中
		Root,         // 4つのボタン
		ConfirmTitle, // 「タイトルへ戻る」の確認
		ConfirmQuit,  // 「ゲーム終了」の確認
	};

private:
	State m_State = State::Closed;
	int m_Selected = 0;

	void Open();
	void Close();

public:
	void Init() override {}
	void Uninit() override {}
	void Update() override;
	void Draw() override {} // 実際の描画はDrawUI() -- 上のクラスコメント参照

	// GameObjectのUpdate()はManager::Update()がManager::IsPaused()==true
	// の間スキップするが、PauseMenu自身はポーズ状態を管理する張本人なので
	// 常に動き続けなければならない。
	bool UpdatesWhilePaused() const override { return true; }

	bool IsOpen() const { return m_State != State::Closed; }

	// Interact::Draw()のHud::Begin()/End()の中から呼んでもらう用。
	void DrawUI();
};
