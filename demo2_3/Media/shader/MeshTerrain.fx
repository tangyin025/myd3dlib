
struct VS_INPUT
{
	float4 Pos				: POSITION;
	float4 Color			: COLOR;
	float2 Tex0				: TEXCOORD0;
	float3 Normal			: NORMAL;
	float3 Tangent			: TANGENT;
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
	return In.Tex0;
}

float3 TransformNormal(VS_INPUT In)
{
	return normalize(mul(In.Normal, (float3x3)g_World));
}

float3 TransformTangent(VS_INPUT In)
{
	return normalize(mul(In.Tangent, (float3x3)g_World));
}

float4 TransformLightWS(VS_INPUT In)
{
	return float4(g_World[3].xyz, length(g_World[0].xyz));
}

float4 TransformColor(VS_INPUT In)
{
	return In.Color;
}
