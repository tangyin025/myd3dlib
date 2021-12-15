
#include "CommonHeader.hlsl"

float4 g_FogColor;
float g_StartDistance = 10;
float g_FogHeight = 50;
float g_Falloff = 0.02;

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
	float Depth = -PosVS.z - g_StartDistance;
	clip(Depth);
	float Height = g_FogHeight - (PosVS.x * g_View._21 + PosVS.y * g_View._22 + PosVS.z * g_View._23 + g_Eye.y);
	float ExpFogFactor = saturate(exp2(Height / g_FogHeight * 7) / 128 * Depth * g_Falloff);
    return float4( g_FogColor.xyz, ExpFogFactor );
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
