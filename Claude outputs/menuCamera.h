#pragma once
#include "camera.h"

// STEP29: タイトル/操作説明/リザルト共通の3D背景で使う、演出専用のカメラ。
// Player/マウス入力を一切見ず、固定位置からゆっくり首を振るだけの見た目を
// 作る。Draw()は継承元のCamera::Draw()をそのまま使う(View/Projection行列を
// 積むロジックはメニュー画面でもゲーム中でも共通なので、継承だけで済む)。
class MenuCamera : public Camera
{
private:
	float m_TimeAccum = 0.0f;
public:
	void Init() override;
	void Update() override;
};
