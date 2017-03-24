
#include "CommonHeader.fx"
float g_bias=0.2;
float g_intensity=5;
float g_sample_rad=100;
float g_scale=10;
float2 offs[16] =
{
	{0.707, 0.707},
	{0.707, 0.000},
	{0.354, 0.354},
	{0.354, 0.000},
	{-0.707, 0.707},
	{-0.000, 0.707},
	{-0.354, 0.354},
	{-0.000, 0.354},
	{-0.707, -0.707},
	{-0.707, -0.000},
	{-0.354, -0.354},
	{-0.354, -0.000},
	{0.707, -0.707},
	{0.000, -0.707},
	{0.354, -0.354},
	{0.000, -0.354}
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
	const float d = length(diff)*g_scale;  
	return max(0.0,dot(cnorm,v)-g_bias)*(1.0/(1.0+d))*g_intensity;  
}  

float4 MainPS(VS_OUTPUT In) : COLOR0
{
	float ao = 0;
	float3 p = tex2D(PositionRTSampler, In.TextureUV).xyz;
	float3 n = tex2D(NormalRTSampler, In.TextureUV).xyz;
	for (int i = 0; i < 16; i++)
	{
		ao += doAmbientOcclusion(In.TextureUV,offs[i]*g_sample_rad/g_ScreenDim/p.z,p,n);
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
