//manager.cpp
#include "main.h"
#include "manager.h"
#include "renderer.h"
#include "Input.h"
#include "camera.h"
#include "gameObject.h"
#include "title.h"
#include "result.h"
#include "Game.h"
#include "gameSettings.h"
#include "splash.h" // STEP40
#include <cmath> // STEP51: tanf/atanf for the culling cone

namespace
{
	// STEP51 (culling): see the big comment in this step's notes -- in short,
	// this approximates the camera's rectangular frustum with a circular
	// cone (sized from the WIDER horizontal half-FOV, plus a safety margin,
	// so the cone always contains the real frustum -- this can only skip
	// objects that truly can't be visible, never one that actually is).
	//
	// Cullable objects are treated as a fixed-radius sphere; kCullRadius is
	// one map cell's worth of world units (Map.cpp's CELL_SIZE), generous
	// enough for essentially every prop/decoration in this game. Objects too
	// big for that (walls, the floor) opt out via GameObject::IsCullable().
	const float kCullRadius = 4.0f;
	const float kCullBehindMargin = 4.0f; // extra world units behind the camera plane still treated as "in view"
	const float kCullHalfFovMargin = 0.35f; // radians (~20 degrees) of slack added on top of the real half-FOV

	float CullHalfFovTan()
	{
		// same derivation Camera::Draw() feeds XMMatrixPerspectiveFovLH: the
		// projection's vertical half-FOV, widened to horizontal by the aspect
		// ratio (screen is wider than tall, so horizontal is the bigger one).
		float halfFovY = Camera::kFovY * 0.5f;
		float halfFovXReal = atanf(tanf(halfFovY) * ((float)SCREEN_WIDTH / (float)SCREEN_HEIGHT));
		return tanf(halfFovXReal + kCullHalfFovMargin);
	}

	bool IsOutsideViewCone(Vector3 objectPosition, Vector3 cameraPosition, Vector3 cameraForward)
	{
		Vector3 toObject = objectPosition - cameraPosition;

		float forwardDist = Vector3::dot(toObject, cameraForward);

		// well behind the camera -- never visible
		if (forwardDist < -(kCullRadius + kCullBehindMargin))
			return true;

		// beyond the far clip plane -- the GPU wouldn't draw it either way
		if (forwardDist > Camera::kFarClip + kCullRadius)
			return true;

		float dist2 = Vector3::dot(toObject, toObject);

		// camera is inside (or right next to) the object's bounding sphere -- always draw
		if (dist2 <= kCullRadius * kCullRadius)
			return false;

		// lateral offset from the forward axis (Pythagoras on the orthogonal forward/lateral split)
		float lateral2 = dist2 - forwardDist * forwardDist;
		if (lateral2 < 0.0f) lateral2 = 0.0f; // guard against float error when forwardDist ~= sqrt(dist2)

		float allowedLateral = forwardDist * CullHalfFovTan() + kCullRadius;

		return lateral2 > allowedLateral * allowedLateral;
	}
}

std::list<GameObject*> Manager::g_GameObject;//リストを使用する場合は、配列ではなくリストを宣言する必要があります。
Scene* Manager::m_Scene = nullptr;
Scene* Manager::m_NextScene = nullptr;
float Manager::m_ChangeTime = 0.0f;
float Manager::m_ChangeTotalTime = 0.0f; // STEP37
float Manager::m_FadeInTimer = 0.0f;     // STEP37
float Manager::m_FadeInDuration = 0.0f;  // STEP37
bool Manager::m_Paused = false;

void Manager::Init()
{
	// STEP24: アプリ起動時に一度だけ -- settings.iniの読み込みを試みる。
	// まだLight/Audioは何も存在しない段階なので、値を保持するだけでよい
	// (各システムへの反映は、それぞれが生成されるタイミングで自分から
	// GameSettingsを読みに行く -- light.cpp Light::Init()等参照)。
	GameSettings::Init();

	Renderer::Init();
	Input::Init();
	// STEP40: 起動直後はいきなりTitleではなく、制作者ロゴスプラッシュ(Splash)を持つ。
	// SplashはSplashLogo::Update()内で自分でChangeScene<Title>()を呼ぶ(splashLogo.cpp参照)。
	ChangeScene<Splash>();
	ChangeScene<result>();
}

void Manager::Uninit()
{
	// STEP24: 追加仕様書13項「設定値はアプリ終了時にファイルへ保存」。
	GameSettings::Save();

	if(m_Scene != nullptr)
	{
		m_Scene->Uninit();
		delete m_Scene;
	}

	for(GameObject* gameObject : g_GameObject)
	{
		gameObject->Uninit();
		delete gameObject;
	}
	g_GameObject.clear();
	Renderer::Uninit();
	Input::Uninit();
}

void Manager::Update()
{
	float dt = 1.0f / 60.0f;

	Input::Update();

	if (m_Scene != nullptr)
	{
		m_Scene->Update();
	}

	// アクティブなオブジェクトのみ更新
	// STEP24: ポーズ中(m_Paused)はUpdatesWhilePaused()がtrueのオブジェクト
	// (PauseMenu/SettingsScreen自身)だけを動かし、それ以外(Player/Camera/
	// Horrorのアンビエントタイマー等)は完全に止める -- 追加仕様書19項
	// 「ポーズ中はゲームロジックが進行しないよう、Update呼び出しを確実に
	// 止める」に対応。
	for (GameObject* gameObject : g_GameObject)
	{
		if (gameObject->GetActive() && (!m_Paused || gameObject->UpdatesWhilePaused()))
		{
			gameObject->Update();
		}
	}

	// Destroy対象削除
	for (auto it = g_GameObject.begin(); it != g_GameObject.end(); )
	{
		if ((*it)->IsDestroy())
		{
			(*it)->Uninit();
			delete (*it);
			it = g_GameObject.erase(it);
		}
		else
		{
			++it;
		}
	}

	if (m_NextScene != nullptr)
	{
		m_ChangeTime -= dt;

		if (m_ChangeTime < 0.0f)
		{
			if (m_Scene != nullptr)
			{
				m_Scene->Uninit();
				delete m_Scene;
			}

			for (GameObject* gameObject : g_GameObject)
			{
				gameObject->Uninit();
				delete gameObject;
			}

			g_GameObject.clear();

			// STEP24: シーンが変わった直後にポーズ状態を持ち越さない安全策
			// (通常はPauseMenu::Update()の「タイトルへ戻る」自身が
			// SetPaused(false)してからChangeSceneするが、念のための保険)。
			m_Paused = false;

			// STEP37: 切り替えた瞬間は画面が真っ黒(フェードアウト完了)の
			// はずなので、ここからm_ChangeTotalTime秒かけて元に戻す
			// (フェードイン)。合計時間はこの時点でスナップショットして
			// おき、フェードイン中に別のChangeScene<T>()が呼ばれて
			// m_ChangeTotalTimeが上書きされても影響を受けないようにする。
			m_FadeInDuration = m_ChangeTotalTime;
			m_FadeInTimer = m_ChangeTotalTime;

			m_Scene = m_NextScene;
			m_Scene->Init();

			m_NextScene = nullptr;
		}
	}
	else if (m_FadeInTimer > 0.0f)
	{
		m_FadeInTimer -= dt;
		if (m_FadeInTimer < 0.0f) m_FadeInTimer = 0.0f;
	}
}

float Manager::GetFadeAlpha()
{
	// フェードアウト中(次のシーンへ切り替わる前): 0(素通し)→1(真っ黒)
	if (m_NextScene != nullptr)
	{
		if (m_ChangeTotalTime <= 0.0001f) return 0.0f; // Time=0指定は「フェード無しの瞬時切り替え」のまま
		float t = 1.0f - (m_ChangeTime / m_ChangeTotalTime);
		if (t < 0.0f) t = 0.0f;
		if (t > 1.0f) t = 1.0f;
		return t;
	}

	// フェードイン中(切り替わった直後): 1(真っ黒)→0(素通し)
	if (m_FadeInTimer > 0.0f && m_FadeInDuration > 0.0001f)
	{
		float t = m_FadeInTimer / m_FadeInDuration;
		if (t < 0.0f) t = 0.0f;
		if (t > 1.0f) t = 1.0f;
		return t;
	}

	return 0.0f;
}

void Manager::Draw()
{
	Renderer::Begin();

	Camera* camera = GetGameObject<Camera>();

	// STEP51: declared here (not inside the `if (camera)` below) so the
	// culling test in the layer loop further down -- which is outside that
	// block -- can still read the camera's position/forward from this frame.
	Vector3 forward;
	Vector3 position;

	if (camera)
	{
		camera->Draw();

		forward = camera->GetForward();
		position = camera->GetPosition();

		for (GameObject* gameObject : g_GameObject)
		{
			gameObject->CalCameraZ(position, forward);
		}

		// Zソート
		g_GameObject.sort([](GameObject* a, GameObject* b)
			{
				return a->GetCameraZ() > b->GetCameraZ();
			});
	}

	for (int layer = 0; layer <= 10; layer++)
	{
		if (layer == 10)
		{
			// STEP48: layer 10 is Hud/TitleMenu/PauseMenu/SettingsScreen/etc --
			// all Direct2D, drawn straight onto the real backbuffer, always at
			// native window resolution. Everything below (layers 0-9: the 3D
			// scene + Polygon2D) has been drawing into the offscreen scene
			// target at the internal render resolution instead; stretch it
			// onto the real backbuffer now, right before layer 10 starts, so
			// the UI/text drawn after this is never softened by the upscale.
			Renderer::BlitSceneToBackBuffer();
		}

		for (GameObject* gameObject : g_GameObject)
		{
			if (gameObject->GetLayer() == layer &&
				gameObject->GetActive())
			{
				// STEP51: skip the Draw() call entirely for objects the
				// culling test can prove aren't visible. Layer 10 (Direct2D
				// UI) is never culled -- it isn't camera/world-space at all.
				if (camera && layer != 10 && gameObject->IsCullable() &&
					IsOutsideViewCone(gameObject->GetPosition(), position, forward))
				{
					continue;
				}

				gameObject->Draw();
			}
		}
	}
	Renderer::End();
}