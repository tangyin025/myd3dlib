
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
    float3 ColorOrig = tex2D( OpaqueRTSampler, In.TextureUV );
	return float4(ColorOrig,1);
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
