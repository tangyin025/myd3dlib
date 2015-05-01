
struct VS_INPUT
{
	float4 Pos				: POSITION;
	float2 Tex0				: TEXCOORD0;
	float3 Normal			: NORMAL;
};

float4 TransformPosWS(VS_INPUT In)
{
	return mul(In.Pos, g_World);
}

float4 TransformPos(VS_INPUT In)
{
	return mul(TransformPosWS(In), g_ViewProj);
}

float2 TransformUV(VS_INPUT In)
{
	return In.Tex0;
}

float3 TransformNormal(VS_INPUT In)
{
	return normalize(mul(In.Normal, (float3x3)g_World));
}
