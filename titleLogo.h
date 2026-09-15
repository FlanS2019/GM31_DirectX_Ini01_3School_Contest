#pragma once
#include "gameObject.h"

class Polygon2D;

// STEP40: タイトル画面用の「廃墟」ロゴ画像(logo_title.png)をフェードイン
// させるだけの薄いラッパー。実際の描画はPolygon2D自身が担当する
// (Manager::AddGameObject<Polygon2D>()で生成し、そのポインタだけ保持して
// 毎フレームSetAlpha()を呼ぶ -- polygon2d.h/.cpp参照)。フェードアウトは
// 個別に行わず、シーン遷移時の全画面フェード(Manager::GetFadeAlpha())に
// 任せる(Titleを抜けるとPolygon2Dごとg_GameObjectから破棄されるので、
// 後始末も不要)。
class TitleLogo : public GameObject
{
private:
	Polygon2D* m_Logo = nullptr;
	float m_Timer = 0.0f;
public:
	void Init() override;
	void Update() override;
};
