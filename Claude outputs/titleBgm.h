#pragma once

#include "gameObject.h"

// STEP28: タイトル画面用のBGM。既存のBgmPlayer(Game::Init()側で
// "abandoned_hospital.mp3"を固定で鳴らすクラス)はInit()がGameObject側の
// 仮想関数のオーバーライドで引数を取れず(Manager::AddGameObject<T>()が
// Init()を引数無しで呼ぶ -- gameObject.h参照)、別のファイルを鳴らす
// バリエーションを増やせない。そのためタイトル用に同じ形のクラスを
// もう1つ用意した。
class TitleBgm : public GameObject
{
private:
	class Audio* m_Bgm = nullptr;

public:
	void Init() override;
	void Uninit() override;
	void Update() override {}
	void Draw() override {}
};
