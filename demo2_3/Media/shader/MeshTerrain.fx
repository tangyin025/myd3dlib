
struct VS_INPUT
{
	uint4 Tex0				: TEXCOORD0;
};

float4 TransformPosWS(VS_INPUT In)
{
	float4 pos = float4((float)In.Tex0.x, 0, (float)In.Tex0.y, 1);
	return mul(pos, g_World);
}

float4 TransformPos(VS_INPUT In)
{
	return mul(TransformPosWS(In), g_ViewProj);
}

float2 TransformUV(VS_INPUT In)
{
	return float2((float)In.Tex0.x / 65.0, (float)In.Tex0.y / 65.0);
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
