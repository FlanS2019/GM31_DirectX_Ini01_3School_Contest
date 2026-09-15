#pragma once
#include "gameObject.h"
#include <vector>
#include <functional>

// STEP40: Game::Init()が今まで1フレームで一気にやっていた重いオブジェクト
// 生成(Camera/Light/Field/Player/Map/Score/BgmPlayer/Interact/PauseMenu/
// SettingsScreen/MenuSound/Horror/GameStartText の13個。Manager::AddGameObject<T>()
// は呼んだ瞬間にT::Init()を同期実行する -- manager.h参照)を、1フレームに
// 1ステップずつ実行するように分割し、進捗(%)とラベルを画面に出す
// ローディング画面。全ステップ完了で自分自身をSetDestroy(true)する。
class LoadingScreen : public GameObject
{
private:
	struct Step
	{
		std::function<void()> Action;
		const char* Label;
	};

	std::vector<Step> m_Steps;
	size_t m_StepIndex = 0;

public:
	void Init() override;
	void Update() override;
	void Draw() override;
};
