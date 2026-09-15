#pragma once
#include "Scene.h"

// STEP29: タイトル/操作説明/リザルトの3画面で共通の3D背景(カメラ+ライト+
// 床+マップ)をまとめた土台クラス。各シーンはこれを継承し、自分の
// Init()の中で最初にMenuBackgroundScene::Init()を呼ぶだけで同じ背景を
// 使い回せる(継承先のInit()で他のGameObjectを追加で足していけばよい)。
// Update()/Draw()は各シーン側で個別に上書きするので、ここでは空実装。
class MenuBackgroundScene : public Scene
{
public:
	void Init() override;
	void Uninit() override;
	void Update() override {}
	void Draw() override {}
};
