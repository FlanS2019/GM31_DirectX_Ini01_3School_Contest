//score.cpp
#include "Score.h"
#include "main.h"
#include "renderer.h"

void Score::Init()
{
    VERTEX_3D vertex[4];

    vertex[0].Position = XMFLOAT3(0.0f, 0.0f, 0.0f);
    vertex[0].TexCoord = XMFLOAT2(0.0f, 0.0f);

    vertex[1].Position = XMFLOAT3(50.0f, 0.0f, 0.0f);
    vertex[1].TexCoord = XMFLOAT2(1.0f, 0.0f);

    vertex[2].Position = XMFLOAT3(0.0f, 50.0f, 0.0f);
    vertex[2].TexCoord = XMFLOAT2(0.0f, 1.0f);

    vertex[3].Position = XMFLOAT3(50.0f, 50.0f, 0.0f);
    vertex[3].TexCoord = XMFLOAT2(1.0f, 1.0f);

    for (int i = 0; i < 4; i++)
    {
        vertex[i].Normal =
            XMFLOAT3(0, 0, -1);

        vertex[i].Diffuse =
            XMFLOAT4(1, 1, 1, 1);
    }

    //頂点バッファ作成
    D3D11_BUFFER_DESC bd{};
    bd.Usage = D3D11_USAGE_DYNAMIC; // ←修正
    bd.ByteWidth = sizeof(VERTEX_3D) * 4;
    bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE; // ←修正

    D3D11_SUBRESOURCE_DATA sd{};
    sd.pSysMem = vertex;

    Renderer::GetDevice()->CreateBuffer(
        &bd,
        &sd,
        &m_VertexBuffer
    );


    //シェーダ
    Renderer::CreateVertexShader(
        &m_VertexShader,
        &m_VertexLayout,
        "shaderunlitTextureVS.cso"
    );

    Renderer::CreatePixelShader(
        &m_PixelShader,
        "shaderunlitTexturePS.cso"
    );


    //テクスチャ
    TexMetadata metadata{};
    ScratchImage image{};

    LoadFromWICFile(
        L"texture\\number_2.png",
        WIC_FLAGS_NONE,
        &metadata,
        image
    );

    CreateShaderResourceView(
        Renderer::GetDevice(),
        image.GetImages(),
        image.GetImageCount(),
        metadata,
        &m_Texture
    );

    m_Value = 0;
}
void Score::Uninit()
{
    if (m_VertexBuffer)
        m_VertexBuffer->Release();

    if (m_VertexLayout)
        m_VertexLayout->Release();

    if (m_VertexShader)
        m_VertexShader->Release();

    if (m_PixelShader)
        m_PixelShader->Release();

    if (m_Texture)
        m_Texture->Release();
}

void Score::Update()
{
}

void Score::Draw()
{
    Renderer::GetDeviceContext()->IASetInputLayout(m_VertexLayout);

    Renderer::GetDeviceContext()->VSSetShader(m_VertexShader, NULL, 0);
    Renderer::GetDeviceContext()->PSSetShader(m_PixelShader, NULL, 0);
    Renderer::GetDeviceContext()->PSSetShaderResources(0, 1, &m_Texture);

    Renderer::SetWorldViewProjection2D();

    MATERIAL material{};
    material.Diffuse = XMFLOAT4(1, 1, 1, 1);
    material.TextureEnable = TRUE;
    Renderer::SetMaterial(material);

    UINT stride = sizeof(VERTEX_3D);
    UINT offset = 0;
    Renderer::GetDeviceContext()->IASetVertexBuffers(0, 1, &m_VertexBuffer, &stride, &offset);
    Renderer::GetDeviceContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

    // ----- 4桁に分解（0~9999にクランプ） -----
    int value = m_Value;
    if (value < 0)    value = 0;
    if (value > 9999) value = 9999;

    int digits[4] =
    {
        (value / 1000) % 10, // 千の位
        (value / 100) % 10, // 百の位
        (value / 10) % 10, // 十の位
        (value) % 10, // 一の位
    };

    for (int d = 0; d < 4; d++)
    {
        // ---- ワールド行列（桁ごとにX方向へオフセット） ----
        XMMATRIX world, scale, rot, trans;

        scale = XMMatrixScaling(m_Scale.x, m_Scale.y, m_Scale.z);
        rot = XMMatrixRotationRollPitchYaw(m_Rotation.x, m_Rotation.y, m_Rotation.z);
        trans = XMMatrixTranslation(
            m_Position.x + d * 50.0f, // 1桁分=50.0fずらす（頂点サイズと合わせる）
            m_Position.y,
            m_Position.z
        );

        world = scale * rot * trans;
        Renderer::SetWorldMatrix(world);

        // ---- 該当する桁の数字に対応するUVを書き込む ----
        int num = digits[d];

        D3D11_MAPPED_SUBRESOURCE msr{};
        HRESULT hr = Renderer::GetDeviceContext()->Map(
            m_VertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &msr
        );

        if (FAILED(hr))
        {
            continue; // この桁だけスキップ
        }

        VERTEX_3D* vertex = (VERTEX_3D*)msr.pData;

        float tx = 1.0f / 5.0f * (num % 5);
        float ty = 1.0f / 5.0f * (num / 5);
        float tw = 1.0f / 5.0f;
        float th = 1.0f / 5.0f;

        vertex[0].Position = XMFLOAT3(0, 0, 0);
        vertex[1].Position = XMFLOAT3(50, 0, 0);
        vertex[2].Position = XMFLOAT3(0, 50, 0);
        vertex[3].Position = XMFLOAT3(50, 50, 0);

        vertex[0].TexCoord = XMFLOAT2(tx, ty);
        vertex[1].TexCoord = XMFLOAT2(tx + tw, ty);
        vertex[2].TexCoord = XMFLOAT2(tx, ty + th);
        vertex[3].TexCoord = XMFLOAT2(tx + tw, ty + th);

        for (int i = 0; i < 4; i++)
        {
            vertex[i].Normal = XMFLOAT3(0, 0, -1);
            vertex[i].Diffuse = XMFLOAT4(1, 1, 1, 1);
        }

        Renderer::GetDeviceContext()->Unmap(m_VertexBuffer, 0);

        // ---- この桁を描画 ----
        Renderer::GetDeviceContext()->Draw(4, 0);
    }
}