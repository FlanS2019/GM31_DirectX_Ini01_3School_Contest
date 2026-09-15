#pragma once

#include "gameObject.h"

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
	void Draw() override {}

	bool UpdatesWhilePaused() const override { return true; }

	bool IsOpen() const { return m_State != State::Closed; }

	void DrawUI();
};
