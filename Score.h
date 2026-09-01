#pragma once
#include <d3d11.h>
#include "gameObject.h"

class Score : public GameObject
{
private:

	ID3D11Buffer* m_VertexBuffer;

	ID3D11InputLayout* m_VertexLayout;
	ID3D11VertexShader* m_VertexShader;
	ID3D11PixelShader* m_PixelShader;

	ID3D11ShaderResourceView* m_Texture;

	int m_Value;

public:
	void Init()override;
	void Uninit()override;
	void Update()override;
	void Draw()override;

	void AddScore(int add) { m_Value += add; }
	// DEBUG: lets other code (Player.cpp, while we're hunting the wall bug)
	// drive this display directly -- currently used to show the live frame
	// counter on-screen instead of the real score, so a screen recording
	// can be matched frame-for-frame against the [COLLIDE]/Output-window
	// log just by reading the number in the corner. Harmless to keep once
	// the bug's fixed; just stop calling it from Player.cpp.
	void SetValue(int value) { m_Value = value; }
	int  GetValue() const { return m_Value; }

};