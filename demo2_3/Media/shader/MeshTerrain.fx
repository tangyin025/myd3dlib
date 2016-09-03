
struct VS_INPUT
{
	uint4 Tex0				: TEXCOORD0;
};

float3 g_TerrainScale;
float4 g_WrappedUV;
uint3 g_ChunkId;
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
	int3 coord = int3(g_ChunkId.x * g_ChunkId.z + In.Tex0.x, g_ChunkId.y * g_ChunkId.z + In.Tex0.y, 0);
	float4 pos = float4(
		g_TerrainScale.x * coord.x,
		g_TerrainScale.y * tex2Dlod(HeightTextureSampler, float4(
			coord.y / g_WrappedUV.w, coord.x / g_WrappedUV.z, 0, 0)).r * 255,
		g_TerrainScale.z * coord.y, 1.0);
	return mul(pos, g_World);
}

float4 TransformPos(VS_INPUT In)
{
	return mul(TransformPosWS(In), g_ViewProj);
}

float2 TransformUV(VS_INPUT In)
{
	return float2(
		(float)In.Tex0.x / g_ChunkId.z * g_WrappedUV.x, (float)In.Tex0.y / g_ChunkId.z * g_WrappedUV.y);
}

float3 TransformNormal(VS_INPUT In)
{
	return normalize(float3(0,1,0));
}

float3 TransformTangent(VS_INPUT In)
{
	return normalize(float3(1,0,0));
}

float4 TransformLight(VS_INPUT In)
{
	return float4(0,0,0,0);
}

float4 TransformColor(VS_INPUT In)
{
	return g_MeshColor;
}
