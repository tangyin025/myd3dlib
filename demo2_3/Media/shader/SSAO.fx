
#include "CommonHeader.fx"

float2 offs[4] =
{
	{1,0},
	{-1,0},
	{0,1},
	{0,-0},
};

struct VS_OUTPUT
{
    float4 Position   : POSITION;   // vertex position 
    float2 TextureUV  : TEXCOORD0;  // vertex texture coords 
};

float doAmbientOcclusion(in float2 tcoord,in float2 uv, in float3 p, in float3 cnorm)
{
	float3 diff = tex2D(PositionRTSampler, tcoord + uv).xyz - p;  
	const float3 v = normalize(diff);  
	const float d = length(diff);  
	return max(0.0,dot(cnorm,v))*(1.0/(1.0+d));  
}  

float4 MainPS(VS_OUTPUT In) : COLOR0
{
	float ao = 0;
	float3 p = tex2D(PositionRTSampler, In.TextureUV).xyz;
	float3 n = tex2D(NormalRTSampler, In.TextureUV).xyz;
	for (int i = 0; i< 4;i++)
	{
		ao += doAmbientOcclusion(In.TextureUV,offs[i]*20/g_ScreenDim/p.z,p,n);
		ao += doAmbientOcclusion(In.TextureUV,offs[i]*40/g_ScreenDim/p.z,p,n);
		ao += doAmbientOcclusion(In.TextureUV,offs[i]*60/g_ScreenDim/p.z,p,n);
		ao += doAmbientOcclusion(In.TextureUV,offs[i]*80/g_ScreenDim/p.z,p,n);
	}
	return 1.0 - ao / 16;
}

technique RenderScene
{
    pass P0
    {
		VertexShader = null;
		PixelShader = compile ps_3_0 MainPS();
	}
}
