
#include "CommonHeader.hlsl"

float4 g_FogColor = { 1, 1, 1, 1 };
float4 g_FogParams = { 10, 50, 0, 0 };

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
    return float4(g_FogColor.xyz, saturate((-PosVS.z - g_FogParams.x) / (g_FogParams.y - g_FogParams.x)));
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
