#pragma once

#include <vector>

class Audio;

// STEP24: 設定画面のBGM/SE音量スライダーを、実際に鳴っているAudio
// コンポーネントへ反映するための小さな中央レジストリ。
//
// Audio::SetVolume()はSourceVoiceのパラメータなので再生中のボイスにも
// 即座に効く(audio.cppのコメント参照)。なので各Audio所有クラスは
// Load()直後にRegisterBgm()/RegisterSe()で「これまで個別に調整してきた
// 基準音量」(歩く音の1.6f、電気のチカチカ音の0.4f等)を登録しておくだけで
// よく、Uninit()の直前にUnregister()で外す。GameSettingsのBGM/SE音量が
// 変わるたびApplyVolumes()が全登録済みAudioに base * 現在の音量設定を
// SetVolume()し直す。
//
// 各SEの「基準音量」自体はこれまで通りこのクラスの外(Player.cpp等)が
// 決める -- SoundManagerは基準値と設定スライダーを掛け合わせるだけで、
// どの音がどれくらい大きいべきかの判断には関与しない。
class SoundManager
{
private:
	struct Entry
	{
		Audio* audio;
		float baseVolume;
	};

	static std::vector<Entry> s_BgmEntries;
	static std::vector<Entry> s_SeEntries;

	// STEP24: ポーズ中の減衰(追加仕様書15項「アンビエントSEは音量を
	// 落として鳴らし続ける」)。SE区分にのみ掛かる -- BGMはポーズ中も
	// 通常音量のまま(ユーザーが設定画面で調整する対象)。
	static float s_PauseAttenuation;

public:
	static void RegisterBgm(Audio* audio, float baseVolume = 1.0f);
	static void RegisterSe(Audio* audio, float baseVolume = 1.0f);

	// 登録されていなくても安全に呼べる(何もしない)。
	static void Unregister(Audio* audio);

	// GameSettingsのBGM/SE音量が変わるたび、またSetPauseAttenuation()から
	// 呼ばれる。登録済みの全Audioに base * 現在の設定を反映する。
	static void ApplyVolumes();

	// PauseMenuがポーズの開始/終了時に呼ぶ(1.0=通常, 0.5=ポーズ中など)。
	static void SetPauseAttenuation(float attenuation);
};
