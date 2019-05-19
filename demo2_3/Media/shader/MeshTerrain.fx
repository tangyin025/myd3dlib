
struct VS_INPUT
{
	int4 Tex0				: TEXCOORD0;
};

float g_HeightScale;
float2 g_HeightTexSize;
int2 g_ChunkId;
int g_ChunkSize;
float2 g_UvRepeat;
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
	int2 Pos = int2(g_ChunkId.y * g_ChunkSize + In.Tex0.y, g_ChunkId.x * g_ChunkSize + In.Tex0.x);
	float4 Color = tex2Dlod(HeightTextureSampler, float4(Pos.x / g_HeightTexSize.x, Pos.y / g_HeightTexSize.y, 0, 0));
	float Height = g_HeightScale * Color.a * 255;
	return mul(float4(Pos.x, Height, Pos.y, 1.0), g_World);
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
	return float2((float)In.Tex0.y / g_ChunkSize, (float)In.Tex0.x / g_ChunkSize);
}

float3 TransformNormal(VS_INPUT In)
{
	int2 Pos = int2(g_ChunkId.y * g_ChunkSize + In.Tex0.y, g_ChunkId.x * g_ChunkSize + In.Tex0.x);
	float4 Color = tex2Dlod(HeightTextureSampler, float4(Pos.x / g_HeightTexSize.x, Pos.y / g_HeightTexSize.y, 0, 0));
	float3 Normal = Color.rgb * 2 - 1;
	return normalize(mul(Normal, (float3x3)g_World));
}

float3 TransformTangent(VS_INPUT In)
{
	int2 Pos = int2(g_ChunkId.y * g_ChunkSize + In.Tex0.y, g_ChunkId.x * g_ChunkSize + In.Tex0.x);
	float4 Color = tex2Dlod(HeightTextureSampler, float4(Pos.x / g_HeightTexSize.x, Pos.y / g_HeightTexSize.y, 0, 0));
	float3 Normal = Color.rgb * 2 - 1;
	float3 Tangent = cross(Normal, float3(0,0,1));
	return normalize(mul(Tangent, (float3x3)g_World));
}

float4 TransformLightWS(VS_INPUT In)
{
	return float4(0,0,0,0);
}

float4 TransformColor(VS_INPUT In)
{
	return float4(1,1,1,1);
}
