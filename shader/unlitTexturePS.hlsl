
#include "common.hlsl"


Texture2D g_Texture : register(t0);
SamplerState g_SamplerState : register(s0);


void main(in PS_IN In, out float4 outDiffuse : SV_Target)
{
    float4 baseColor;

    if (Material.TextureEnable)
    {
        baseColor = g_Texture.Sample(g_SamplerState, In.TexCoord);
        baseColor *= In.Diffuse;
    }
    else
    {
        baseColor = In.Diffuse;
    }

    if (!Light.Enable)
    {
        outDiffuse = baseColor;
        return;
    }

    float3 N = normalize(In.Normal);
    float3 lit;

    if (Light.IsSpot)
    {
        float3 toLight = Light.Position.xyz - In.WorldPos;
        float dist = length(toLight);
        float3 L = toLight / max(dist, 0.0001);

        float NdotL = saturate(dot(N, L));

        float3 spotDir = normalize(Light.Direction.xyz);
        float cosAngle = dot(-L, spotDir);
        float spotFactor = smoothstep(Light.SpotParams.y, Light.SpotParams.x, cosAngle);

        float range = max(Light.SpotParams.z, 0.0001);
        float atten = saturate(1.0 - dist / range);
        atten *= atten; // cheap falloff curve instead of a flat linear ramp

        lit = baseColor.rgb * (Light.Ambient.rgb + Light.Diffuse.rgb * NdotL * spotFactor * atten);
    }
    else
    {
        float3 L = normalize(-Light.Direction.xyz);
        float diffuseTerm = saturate(dot(N, L));
        lit = baseColor.rgb * (Light.Ambient.rgb + Light.Diffuse.rgb * diffuseTerm);
    }

    outDiffuse = float4(lit, baseColor.a);
}