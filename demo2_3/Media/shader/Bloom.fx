
#include "CommonHeader.hlsl"

//--------------------------------------------------------------------------------------
// Vertex shader output structure
//--------------------------------------------------------------------------------------
struct VS_OUTPUT
{
    float4 Position   : POSITION;   // vertex position 
    float2 TextureUV  : TEXCOORD0;  // vertex texture coords 
};

float4 MainPS(VS_OUTPUT In) : COLOR0
{
	return float4(1,0,0,1);
}

//--------------------------------------------------------------------------------------
// Renders scene 
//--------------------------------------------------------------------------------------
technique RenderScene
{
    pass P0
    {
		VertexShader = null;
		PixelShader = compile ps_3_0 MainPS();
	}
}
