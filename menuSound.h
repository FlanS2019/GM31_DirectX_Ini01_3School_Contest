#pragma once

#include "gameObject.h"

// STEP27: メニュー操作(カーソル移動/決定)にSEを付ける要望への対応。
// TitleMenu/PauseMenu/SettingsScreen/ResultMenu/HowToPlayMenuの全部から
// 共通で使うので、SettingsScreenと同じ「各シーンのInit()で自分用の
// インスタンスを1つ追加してもらい、使う側はManager::GetGameObject<
// MenuSound>()で参照するだけ」というパターンにしてある(1つのインスタンス
// をシーンをまたいで使い回すことはできないため -- settingsScreen.hの
// クラスコメント参照)。
//
// ファイル名は既存SE("Heartbeat03-1(Slow-Reverb).mp3"等、audio\SE
// フォルダ)と同じ命名規則のはず、という前提で書いてある。実際の
// ファイル名が違う場合はmenuSound.cppの2箇所のLoad()を差し替えるだけでよい。
class MenuSound : public GameObject
{
private:
	class Audio* m_MoveSE = nullptr;
	class Audio* m_ConfirmSE = nullptr;

public:
	void Init() override;
	void Uninit() override;
	void Update() override {}
	void Draw() override {}

	// カーソル移動(W/S/上下キーでの選択項目切り替え)時に呼ぶ。
	void PlayMove();

	// 決定(Enterでの実行/確定)時に呼ぶ。
	void PlayConfirm();
};
