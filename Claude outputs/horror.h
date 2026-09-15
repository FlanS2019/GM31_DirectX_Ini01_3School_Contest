#pragma once

#include "gameObject.h"

class Audio;

// STEP15: 「画面エフェクト強化 / サウンド・ジャンプスケア / 暗闇・懐中電灯
// 演出」の3本立てをまとめる小さなシングルトン(Light/Interactと同じく
// Game.cppでAddGameObject<Horror>()するだけの子)。既存のLightの懐中電灯
// (STEP5、light.h/.cpp)には一切手を入れず、IsFlashlightOn()を覗くだけ --
// 懐中電灯がOFFの間だけ心臓の鼓動ループ(Heartbeat03-3(Slow-Loop).mp3)を
// 鳴らし、Hud::DrawVignette()で画面の縁を常時薄暗く縁取る(OFF中はさらに
// 濃く)。ジャンプスケアはTriggerJumpScare()を呼ぶだけで発動 -- 悲鳴っぽい
// SEを1回再生 + 画面を一瞬白くフラッシュ(Hud::DrawFullScreenTint)させる。
// 呼び出し側(itemBox.cppなど)はAudio/Hudの存在を知らなくていい。
//
// 描画だけは特殊: Direct2DはBeginDraw/EndDrawの対がフレームに1組だけという
// 制約があるので、Horror::Draw()そのものは空のままにして(Managerの自動
// 描画に任せない)、既にHud::Begin()/End()で挟んでいるInteract::Draw()から
// DrawScreenEffects()を明示的に呼んでもらう形にした -- interact.cpp参照。
class Horror : public GameObject
{
private:
	Audio* m_Heartbeat = nullptr;
	bool m_HeartbeatPlaying = false;

	Audio* m_ScareSting = nullptr;
	float m_FlashTimer = 0.0f;
	const float m_FlashDuration = 0.35f;

public:
	void Init() override;
	void Uninit() override;
	void Update() override;
	void Draw() override {} // 実際の画面効果はDrawScreenEffects()側 -- 上のコメント参照

	// ItemBox::Interact()のような「ここぞ」という場所から1回呼ぶだけでいい。
	// 既にフラッシュ中でも安全(単に最初からやり直すだけで、重ねがけはしない)。
	void TriggerJumpScare();

	// Interact::Draw()のHud::Begin()/End()の中から呼んでもらう用 -- 常時
	// ヴィネットを描き、フラッシュ中なら白いティントも重ねる。
	void DrawScreenEffects();
};
