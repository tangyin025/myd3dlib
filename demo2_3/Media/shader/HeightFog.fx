
#include "CommonHeader.fx"

float4 g_FogColor;
float g_StartDistance = 10;
float g_FogHeight = 10;

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
	float4 ViewPos = tex2D(PositionRTSampler, In.TextureUV);
	float Depth = -ViewPos.z - g_StartDistance;
	clip(Depth);
	float Height = g_FogHeight - (ViewPos.x * g_View._21 + ViewPos.y * g_View._22 + ViewPos.z * g_View._23 + g_Eye.y);
	clip(Height);
	float ExpFogFactor = saturate(exp2(Height / g_FogHeight * 7) / 128);
    return float4( g_FogColor.xyz,ExpFogFactor );
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
