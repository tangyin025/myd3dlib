
#include "CommonHeader.hlsl"

struct VS_OUTPUT
{
    float4 Position   : POSITION;   // vertex position 
    float2 TextureUV  : TEXCOORD0;  // vertex texture coords 
};

float4 MainPS(VS_OUTPUT In) : COLOR0
{
	float3 n = (tex2D(NormalRTSampler, In.TextureUV).xyz + 1) * 0.5;
	return float4(n, 1);
}

technique RenderScene
{
    pass P0
    {
		VertexShader = null;
		PixelShader = compile ps_3_0 MainPS();
	}
}
