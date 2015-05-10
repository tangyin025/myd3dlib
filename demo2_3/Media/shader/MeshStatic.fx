
struct VS_INPUT
{
	float4 Pos				: POSITION;
	float4 Color			: COLOR0;
	float2 Tex0				: TEXCOORD0;
	float3 Normal			: NORMAL;
#ifdef TEXTURE_TYPE_NORMAL
	float3 Tangent			: TANGENT;
#endif
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

#ifdef TEXTURE_TYPE_NORMAL
float3 TransformTangent(VS_INPUT In)
{
	return normalize(mul(In.Tangent, (float3x3)g_World));
}
#endif

float4 TransformLight(VS_INPUT In)
{
	return float4(0,0,0,0);
}

float4 TransformColor(VS_INPUT In)
{
	return In.Color;
}
