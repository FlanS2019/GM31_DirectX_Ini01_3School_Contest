#include "main.h"
#include "Input.h"

BYTE Input::m_OldKeyState[256] = {};
BYTE Input::m_PrevKeyState[256] = {};

float Input::m_MouseDeltaX = 0.0f;
float Input::m_MouseDeltaY = 0.0f;
bool Input::m_MouseCaptureEnabled = false; // STEP36

int Input::m_MouseX = 0;         // STEP37
int Input::m_MouseY = 0;         // STEP37
bool Input::m_MouseLeftDown = false;     // STEP37
bool Input::m_MouseLeftDownPrev = false; // STEP37

void Input::Init()
{
	memset(m_OldKeyState, 0, 256);
	memset(m_PrevKeyState, 0, 256);

	// STEP36: 「タイトルでマウスが使えない」対策 -- 以前はここで無条件に
	// ShowCursor(FALSE)していたため、マウス視点操作が要らないタイトル/
	// 操作説明/設定画面でもカーソルが常に非表示・中央固定になっていた。
	// 既定はキャプチャOFF(通常のカーソルが使える状態)にしておき、
	// 実際にマウス視点操作が要るGame::Init()側でONにする。
	m_MouseCaptureEnabled = false;
}
void Input::Uninit()
{
	ShowCursor(TRUE); // Init()時点でHIDEしていない場合でも無害(カウンタが1増えるだけ)
}
void Input::SetMouseCaptureEnabled(bool enabled)
{
	if (enabled == m_MouseCaptureEnabled) return;
	m_MouseCaptureEnabled = enabled;

	HWND hwnd = GetWindow();

	if (enabled)
	{
		// STEP36: キャプチャ開始時に一度カーソルを中央へ寄せておく。
		// でないと、直前まで自由に動かしていたカーソル位置との差分が
		// 初回フレームでそのまま大きな視点の飛びとして出てしまう。
		RECT rect;
		GetClientRect(hwnd, &rect);
		POINT center = { (rect.right - rect.left) / 2, (rect.bottom - rect.top) / 2 };
		ClientToScreen(hwnd, &center);
		SetCursorPos(center.x, center.y);

		ShowCursor(FALSE);
	}
	else
	{
		ShowCursor(TRUE);
	}

	m_MouseDeltaX = 0.0f;
	m_MouseDeltaY = 0.0f;
}
void Input::Update()
{
	memcpy(m_PrevKeyState, m_OldKeyState, 256);
	GetKeyboardState(m_OldKeyState);

	// STEP37: 「クリックしても反応しない」対策 -- メニューのマウス操作用に
	// クライアント座標と左クリックの押下判定を、キャプチャON/OFFに関わらず
	// 毎フレーム更新しておく(キャプチャON中=ゲームプレイ中はカーソルが
	// 中央固定なのでメニュー側では使われないが、無害)。
	{
		HWND hwnd = GetWindow();

		POINT p;
		GetCursorPos(&p);
		ScreenToClient(hwnd, &p);
		m_MouseX = p.x;
		m_MouseY = p.y;

		m_MouseLeftDownPrev = m_MouseLeftDown;
		m_MouseLeftDown = (GetAsyncKeyState(VK_LBUTTON) & 0x8000) != 0;
	}

	// STEP36: キャプチャOFFの間(タイトル/操作説明/設定/一時停止中など)は
	// カーソルの再センタリングを一切行わない -- 通常のマウスカーソルを
	// そのまま自由に使わせる。
	if (!m_MouseCaptureEnabled)
	{
		m_MouseDeltaX = 0.0f;
		m_MouseDeltaY = 0.0f;
		return;
	}

	// mouse delta measured against the window center, then re-center
	// so the cursor never reaches the screen edge (FPS-style look)
	HWND hwnd = GetWindow();
	RECT rect;
	GetClientRect(hwnd, &rect);
	POINT center = { (rect.right - rect.left) / 2, (rect.bottom - rect.top) / 2 };
	ClientToScreen(hwnd, &center);

	POINT curPos;
	GetCursorPos(&curPos);

	m_MouseDeltaX = (float)(curPos.x - center.x);
	m_MouseDeltaY = (float)(curPos.y - center.y);

	SetCursorPos(center.x, center.y);
}
bool Input::GetKeyPress(BYTE KeyCode)
{
	return (m_OldKeyState[KeyCode] & 0x80) != 0;
}
bool Input::GetKeyTrigger(BYTE KeyCode)
{
	return ((m_OldKeyState[KeyCode] & 0x80) != 0) && ((m_PrevKeyState[KeyCode] & 0x80) == 0);
}