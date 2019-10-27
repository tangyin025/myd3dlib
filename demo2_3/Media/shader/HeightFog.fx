
#include "CommonHeader.fx"

float g_StartDistance;
float4 g_FogColor;

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
	float depth = -(tex2D( PositionRTSampler, In.TextureUV ).z + g_StartDistance);
	clip(depth);
	float alpha = saturate(depth/10);
    return float4( g_FogColor.xyz,alpha );
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
