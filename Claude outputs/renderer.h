#pragma once

#define MAX_POINT_LIGHTS 16

struct VERTEX_3D
{
	XMFLOAT3 Position;
	XMFLOAT3 Normal;
	XMFLOAT4 Diffuse;
	XMFLOAT2 TexCoord;
};

struct MATERIAL
{
	XMFLOAT4	Ambient;
	XMFLOAT4	Diffuse;
	XMFLOAT4	Specular;
	XMFLOAT4	Emission;
	float		Shininess;
	BOOL		TextureEnable;
	float		Dummy[2];
};



struct LIGHT
{
	BOOL		Enable;
	BOOL		IsSpot;     // false = old-style infinite directional light; true = handheld flashlight (STEP5)
	BOOL		Dummy[2];
	XMFLOAT4	Direction;  // directional: the light's direction. spot: the direction it's aimed.
	XMFLOAT4	Diffuse;
	XMFLOAT4	Ambient;
	XMFLOAT4	Position;   // spot light origin (world space); unused when IsSpot is false
	XMFLOAT4	SpotParams; // x = cos(inner cone), y = cos(outer cone), z = range, w unused
};

struct POINT_LIGHT
{
	XMFLOAT4 Position;
	XMFLOAT4 Color;
	XMFLOAT4 Params; // x = range
};

// How many times the diffuse texture repeats across an object's UV, instead
// of stretching. Everything defaults to (1,1) (today's look, unchanged) --
// see Renderer::SetWorldMatrix()/SetUVTiling() and box.cpp's Draw().
struct UV_TILING
{
	float U;
	float V;
	float Pad[2];
};

class Renderer
{
private:

	static D3D_FEATURE_LEVEL       m_FeatureLevel;

	static ID3D11Device*           m_Device;
	static ID3D11DeviceContext*    m_DeviceContext;
	static IDXGISwapChain*         m_SwapChain;
	static ID3D11RenderTargetView* m_RenderTargetView;
	static ID3D11DepthStencilView* m_DepthStencilView;

	static ID3D11Buffer*			m_WorldBuffer;
	static ID3D11Buffer*			m_ViewBuffer;
	static ID3D11Buffer*			m_ProjectionBuffer;
	static ID3D11Buffer*			m_MaterialBuffer;
	static ID3D11Buffer*			m_LightBuffer;

	// STEP42: SetLight()が毎回キャッシュする。Polygon2D::Draw()が自分の描画の間だけ一時的にライトを
	// 無効化(Enable=false)して元に戻すために使う(GetLight()参照)。unlitTexturePS.hlslは
	// 名前に反してLight.Enableがtrueならシーンのライティングをそのまま適用してしまうため、
	// タイトルロゴ/UIのPolygon2Dをこのシェーダーで描くとLightの暗いAmbient(ホラー演出用)の
	// 影響を受けてほぼ見えなくなる(polygon2d.cpp参照)。他の全ての3Dオブジェクト(Player/
	// 壁/boxなど)はこのシェーダーを共有しているのでシェーダー自体は変えず、C++側で一時退避する。
	static LIGHT m_CurrentLight;

	static ID3D11Buffer* m_PointLightBuffer;
	static POINT_LIGHT				m_PendingPointLights[MAX_POINT_LIGHTS];
	static int						m_PendingPointLightCount;
	static ID3D11Buffer*			m_TilingBuffer;


	static ID3D11DepthStencilState* m_DepthStateEnable;
	static ID3D11DepthStencilState* m_DepthStateDisable;

	static ID3D11BlendState*		m_BlendState;
	static ID3D11BlendState*		m_BlendStateATC;

	// STEP49: opaque (BlendEnable=FALSE) blend state, used only by
	// BlitSceneToBackBuffer() -- see its definition in renderer.cpp for why.
	static ID3D11BlendState*		m_BlendStateOpaque;

	// STEP48: internal render resolution. Separate from the window/backbuffer
	// (SCREEN_WIDTH/HEIGHT) -- the 3D scene renders at this size only, then
	// gets stretched onto the backbuffer. See SetInternalResolution() /
	// BlitSceneToBackBuffer() in renderer.cpp.
	static ID3D11Texture2D*          m_SceneColorTexture;
	static ID3D11RenderTargetView*   m_SceneRenderTargetView;
	static ID3D11ShaderResourceView* m_SceneShaderResourceView;
	static ID3D11Texture2D*          m_SceneDepthTexture;
	static ID3D11DepthStencilView*   m_SceneDepthStencilView;
	static int                       m_RenderWidth;
	static int                       m_RenderHeight;

	// STEP48: fullscreen-quad resources for stretching the offscreen scene
	// texture onto the real backbuffer. Reuses the existing unlitTextureVS/
	// PS.cso pair (same shaders/vertex layout Polygon2D already uses for its
	// own screen-space rectangles) -- no new shader needed.
	static ID3D11Buffer*        m_BlitVertexBuffer;
	static ID3D11VertexShader*  m_BlitVertexShader;
	static ID3D11InputLayout*   m_BlitVertexLayout;
	static ID3D11PixelShader*   m_BlitPixelShader;

public:
	static void Init();
	static void Uninit();
	static void Begin();
	static void End();

	static void SetDepthEnable(bool Enable);
	static void SetATCEnable(bool Enable);

	// STEP48: resolutionIndex is 0=144p, 1=360p, 2=480p, 3=1080p, 4=4K (see
	// settingsScreen.cpp's Row_Resolution / gameSettings.cpp). Out-of-range
	// values are clamped. A no-op if already at that resolution.
	static void SetInternalResolution(int resolutionIndex);
	static void BlitSceneToBackBuffer();
	static int GetRenderWidth() { return m_RenderWidth; }
	static int GetRenderHeight() { return m_RenderHeight; }

	static void SetWorldViewProjection2D();
	static void SetWorldMatrix(XMMATRIX WorldMatrix);
	static void SetViewMatrix(XMMATRIX ViewMatrix);
	static void SetProjectionMatrix(XMMATRIX ProjectionMatrix);
	static void SetMaterial(MATERIAL Material);
	static void SetLight(LIGHT Light);
	static LIGHT GetLight() { return m_CurrentLight; } // STEP42
	static void AddPointLight(XMFLOAT3 position, XMFLOAT3 color, float range);
	static void SetUVTiling(float U, float V);

	static ID3D11Device* GetDevice( void ){ return m_Device; }
	static ID3D11DeviceContext* GetDeviceContext( void ){ return m_DeviceContext; }
	static IDXGISwapChain* GetSwapChain(void) { return m_SwapChain; }


	static void CreateVertexShader(ID3D11VertexShader** VertexShader, ID3D11InputLayout** VertexLayout, const char* FileName);
	static void CreatePixelShader(ID3D11PixelShader** PixelShader, const char* FileName);


};
