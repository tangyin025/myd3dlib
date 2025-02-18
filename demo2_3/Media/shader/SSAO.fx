
#include "CommonHeader.hlsl"
float g_bias=0.2;
float g_intensity=5;
float g_sample_rad=100;
float g_scale=10;
texture g_OcclusionRT;

sampler OcclusionRTSampler = sampler_state
{
	Texture = <g_OcclusionRT>;
	MipFilter = NONE;
	MinFilter = POINT;
	MagFilter = POINT;
    ADDRESSU = CLAMP;
    ADDRESSV = CLAMP;
};

float2 ssaoKernel[16] =
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

float2 ssaoNoise[16]=
{
	{0.26733112335205, -0.43318524956703},
	{-0.26811882853508, 0.76381689310074},
	{0.10568914562464, 0.090765230357647},
	{-0.60723882913589, 0.65604424476624},
	{-0.61585539579391, -0.53261595964432},
	{0.45133924484253, -0.59424471855164},
	{0.56987339258194, 0.91667246818542},
	{0.9441967010498, 0.53230553865433},
	{0.70194286108017, -0.62349832057953},
	{0.08718866109848, -0.061237376183271},
	{-0.82041823863983, 0.0050604329444468},
	{-0.022253500297666, -0.65527421236038},
	{0.85587269067764, -0.40318629145622},
	{0.57523638010025, -0.46238535642624},
	{-0.029811553657055, 0.7737962603569},
	{-0.089441284537315, 0.0065105808898807}
};

struct VS_OUTPUT
{
    float4 Position   : POSITION;   // vertex position 
    float2 TextureUV  : TEXCOORD0;  // vertex texture coords 
};

float4 OcclusionPS(VS_OUTPUT In) : COLOR0
{
	// // https://learnopengl.com/Advanced-Lighting/SSAO
	// float3 fragPos = tex2D(PositionRTSampler, In.TextureUV).xyz;
	// float3 normal = tex2D(NormalRTSampler, In.TextureUV).xyz;
	// float3 randomVec = ssaoNoise[In.TextureUV.y * g_ScreenDim.y % 4 * 4 + In.TextureUV.x * g_ScreenDim.x % 4];

	// float3 tangent = normalize(randomVec - normal * dot(randomVec, normal));
	// float3 bitangent = cross(normal, tangent);
	// float3x3 TBN = float3x3(tangent, bitangent, normal);

	// float occlusion = 0;
	// for (int i = 0; i < 16; i++)
	// {
	// 	float3 sample = mul(ssaoKernel[i], TBN);
	// 	sample = fragPos + sample * g_sample_rad;
	// 	float4 offset = float4(sample, 1);
	// 	offset = mul(offset, g_Proj);
	// 	offset.xyz /= offset.w;
	// 	offset.xyz = offset.xyz * 0.5 + 0.5;
	// 	offset.y = 1 - offset.y;

	// 	float sampleDepth = tex2D(PositionRTSampler, offset.xy + 0.5 / g_ScreenDim).z;
	// 	float rangeCheck = smoothstep(0.0, 1.0, g_sample_rad / abs(fragPos.z - sampleDepth));
	// 	occlusion += (sampleDepth >= sample.z + g_bias ? 1.0 : 0.0) * rangeCheck;
	// }

	float3 fragPos = tex2D(PositionRTSampler, In.TextureUV).xyz;
	float3 normal = tex2D(NormalRTSampler, In.TextureUV).xyz;
	float2 noise = ssaoNoise[In.TextureUV.y * g_ScreenDim.y % 4 * 4 + In.TextureUV.x * g_ScreenDim.x % 4];
	float2 aspect = float2(g_sample_rad / 720 / fragPos.z * g_ScreenDim.y / g_ScreenDim.x, g_sample_rad / 720 / fragPos.z);

	float occlusion = 0;
	for (int i = 0; i < 16; i++)
	{
		float2 kernel = ssaoKernel[i] + noise;
		kernel *= aspect;
		float3 offset = tex2D(PositionRTSampler, In.TextureUV + kernel).xyz - fragPos;

		const float3 v = normalize(offset);
		const float d = length(offset) * g_scale;
		occlusion += max(0.0, dot(normal, v) - g_bias) * (1.0 / (1.0 + d)) * g_intensity;
	}
	return float4(1.0 - occlusion / 16, 0, 0, 0);
}

float4 AmbientPS(VS_OUTPUT In) : COLOR0
{
	// return float4(g_AmbientColor.xyz * tex2D(OcclusionRTSampler, In.TextureUV).r, 0);

	float2 texelSize = 1.0 / g_ScreenDim;
    float result = 0.0;
    for (int x = -2; x < 2; ++x) 
    {
        for (int y = -2; y < 2; ++y) 
        {
			float2 offset = float2(x, y) * texelSize;
			result += tex2D(OcclusionRTSampler, offset + In.TextureUV).r;
        }
    }
	return float4(g_AmbientColor.xyz * result / (4.0 * 4.0), 0);
}

technique RenderScene
{
    pass P0
    {
		VertexShader = null;
		PixelShader = compile ps_3_0 OcclusionPS();
	}
    pass P1
    {
		VertexShader = null;
		PixelShader = compile ps_3_0 AmbientPS();
	}
}
