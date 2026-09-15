#pragma once
#include "gameObject.h"

// STEP32: ゲーム開始直後に「ゲームスタート」の文字をふわっと(フェード
// イン→少し表示→フェードアウト)出すだけの演出用オブジェクト。
// Hud::DrawTextAlpha()(STEP32でHudに追加)を使う。表示し終わったら
// SetDestroy(true)で自分自身を消す(Game::Init()で1回追加するだけでいい)。
class GameStartText : public GameObject
{
private:
	float m_Timer = 0.0f;
public:
	void Init() override;
	void Update() override;
	void Draw() override;
};
