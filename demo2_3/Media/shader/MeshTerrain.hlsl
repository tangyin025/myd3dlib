
float2 g_TerrainSize;

struct VS_INPUT
{
	float4 Pos				: POSITION;
	float3 Normal			: NORMAL;
	float4 Color			: COLOR0;
};

float4 TransformPosWS(VS_INPUT In)
{
	return mul(In.Pos, g_World);
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
	return float2(In.Pos.x / g_TerrainSize.x, In.Pos.z / g_TerrainSize.y);
}

float3 TransformNormal(VS_INPUT In)
{
	return normalize(mul(In.Normal, (float3x3)g_World));
}

float3 TransformTangent(VS_INPUT In)
{
	return normalize(mul(cross(In.Normal, float3(0, 0, 1)), (float3x3)g_World));
}

float4 TransformLightWS(VS_INPUT In)
{
	return float4(g_World[3].xyz, length(g_World[0].xyz));
}

float4 TransformColor(VS_INPUT In)
{
	return In.Color;
}
