
struct VS_INPUT
{
	uint4 Tex0				: TEXCOORD0;
};

float g_HeightScale;
float2 g_HeightTexSize;
uint2 g_ChunkId;
uint g_ChunkSize;
Texture2D g_HeightTexture;

sampler HeightTextureSampler = sampler_state
{
	Texture = <g_HeightTexture>;
	MipFilter = NONE;
	MinFilter = POINT;
	MagFilter = POINT;
	AddressU = CLAMP;
	AddressV = CLAMP;
};

float4 TransformPosWS(VS_INPUT In)
{
	int2 coord = int2(g_ChunkId.x * g_ChunkSize + In.Tex0.x, g_ChunkId.y * g_ChunkSize + In.Tex0.y);
	float height = g_HeightScale * tex2Dlod(HeightTextureSampler,
		float4(coord.y / g_HeightTexSize.y, coord.x / g_HeightTexSize.x, 0, 0)).a * 255;
	return mul(float4(coord.x, height, coord.y, 1.0), g_World);
}

float4 TransformPos(VS_INPUT In)
{
	return mul(TransformPosWS(In), g_ViewProj);
}

float4 TransformPosShadow(VS_INPUT In)
{
	return mul(TransformPosWS(In), g_SkyLightViewProj);
}

float2 TransformUV(VS_INPUT In)
{
	return float2(
		(float)In.Tex0.x / g_ChunkSize,
		(float)In.Tex0.y / g_ChunkSize);
}

float3 TransformNormal(VS_INPUT In)
{
	int2 coord = int2(g_ChunkId.x * g_ChunkSize + In.Tex0.x, g_ChunkId.y * g_ChunkSize + In.Tex0.y);
	float3 normal = tex2Dlod(HeightTextureSampler,
		float4(coord.y / g_HeightTexSize.y, coord.x / g_HeightTexSize.x, 0, 0)).rgb * 2 - 1;
	return normalize(mul(normal, (float3x3)g_World));
}

float3 TransformTangent(VS_INPUT In)
{
	int2 coord = int2(g_ChunkId.x * g_ChunkSize + In.Tex0.x, g_ChunkId.y * g_ChunkSize + In.Tex0.y);
	float3 normal = tex2Dlod(HeightTextureSampler,
		float4(coord.y / g_HeightTexSize.y, coord.x / g_HeightTexSize.x, 0, 0)).rgb * 2 - 1;
	float3 Tangent = cross(normal, float3(0,0,1));
	return normalize(mul(Tangent, (float3x3)g_World));
}

float4 TransformLightWS(VS_INPUT In)
{
	return float4(0,0,0,0);
}
