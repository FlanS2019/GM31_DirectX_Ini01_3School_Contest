#pragma once

class GameSettings
{
public:
	static void Init();

	static void Load(); 
	static void Save();
	static void ResetToDefault();

	static float GetBgmVolume();
	static void SetBgmVolume(float volume01);

	static float GetSeVolume();
	static void SetSeVolume(float volume01);

	static float GetBrightness();
	static void SetBrightness(float brightness01);

	static bool GetFullscreen();
	static void SetFullscreen(bool fullscreen);

	static int GetFpsCap();
	static void SetFpsCap(int fpsCap);

	static float GetMouseSensitivity();
	static void SetMouseSensitivity(float sensitivity);

	static bool GetInvertY();
	static void SetInvertY(bool invert);

	static int GetHorrorIntensity();
	static void SetHorrorIntensity(int intensity);
};
