
#include "CommonHeader.hlsl"

float4 g_FogColor = { 0.518, 0.553, 0.608, 1 };
float4 g_FogParams = { 0.01, 0, 0, 0 };

//--------------------------------------------------------------------------------------
// Vertex shader output structure
//--------------------------------------------------------------------------------------
struct VS_OUTPUT
{
    float4 Position   : POSITION;   // vertex position 
    float2 TextureUV  : TEXCOORD0;  // vertex texture coords 
};

float4 HeightFogPS( VS_OUTPUT In ) : COLOR0
{
    float4 PosVS = tex2D(PositionRTSampler, In.TextureUV);
    // https://learn.microsoft.com/en-us/windows/win32/direct3d9/fog-formulas#exponential-fog
    return float4(g_FogColor.xyz, 1 - 1 / exp(-PosVS.z * g_FogParams.x));
}

//--------------------------------------------------------------------------------------
// Renders scene 
//--------------------------------------------------------------------------------------
technique RenderScene
{
    pass P0
    {          
        VertexShader = null;
        PixelShader  = compile ps_3_0 HeightFogPS(); 
    }
}
