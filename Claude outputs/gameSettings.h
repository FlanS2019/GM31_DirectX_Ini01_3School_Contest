#pragma once

// STEP24: 「一時停止・設定画面 追加仕様書」13項の設定値をまとめて持つ
// 静的クラス(Input/Renderer/Managerと同じく、インスタンス化せず全部static)。
// SettingsScreenは「値を持っている」ことだけ知っていればよく、値が変わった
// ときにどのシステム(SoundManager/Light/Camera)へ配線するかはSet*()の中に
// 隠してある。Init()はアプリ起動時に一度だけ(Manager::Init()から)呼ぶ。
class GameSettings
{
public:
	// 起動時に一度。settings.iniの読み込みを試み、失敗時は既定値のまま
	// (「時間の都合で保存機能が間に合わない場合は...その場限りの設定でもよい」
	// -- 追加仕様書13項 -- に対応: ファイルが無くても普通に動く)。
	static void Init();

	static void Load(); // settings.ini から読み込み(無ければ何もしない)
	static void Save(); // settings.ini へ書き出し(Manager::Uninit()から呼ばれる)
	static void ResetToDefault();

	static float GetBgmVolume();
	static void SetBgmVolume(float volume01); // 0..1, SoundManager::ApplyVolumes()まで面倒を見る

	static float GetSeVolume();
	static void SetSeVolume(float volume01);

	// 画面の光量。0..1の素の値をそのまま保持し、暗さの下限を守るための
	// マッピング(0.7～1.3倍等)はLight::SetBrightness01()側の責務にする。
	static float GetBrightness();
	static void SetBrightness(float brightness01);

	// STEP24時点では値の保持のみ。実際のウィンドウモード切り替えは
	// 未実装(追加仕様書19項: 「時間があれば検討」の対象)。
	static bool GetFullscreen();
	static void SetFullscreen(bool fullscreen);

	// 0 = 30fps, 1 = 60fps, 2 = 無制限。こちらも値の保持のみ -- 実際の
	// フレームレート制限(WinMain側のメッセージループ)は未実装。
	static int GetFpsCap();
	static void SetFpsCap(int fpsCap);

	// マウス感度の倍率(1.0が現状のデフォルト挙動と同じ)。Camera::Update()
	// が毎フレーム読みに行くだけなので、こちらにはSet側の追加配線はない。
	static float GetMouseSensitivity();
	static void SetMouseSensitivity(float sensitivity);

	static bool GetInvertY();
	static void SetInvertY(bool invert);

	// 0 = 弱, 1 = 中, 2 = 強。horror.cppがヴィネット/フラッシュの強さと
	// 不定期アンビエントSEの間隔を読みに行く。
	static int GetHorrorIntensity();
	static void SetHorrorIntensity(int intensity);

	// STEP48: 0=144p, 1=360p, 2=480p, 3=1080p, 4=4K -- internal render
	// resolution only (Renderer::SetInternalResolution()); window size is
	// never touched. Default 3 = native 1080p (no upscaling at all).
	static int GetResolutionIndex();
	static void SetResolutionIndex(int index);
};
