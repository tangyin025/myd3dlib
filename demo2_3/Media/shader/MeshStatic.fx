
struct VS_INPUT
{
	float4 Pos				: POSITION;
#if INSTANCE
	float4 Pos1				: POSITION1;
	float4 Pos2				: POSITION2;
	float4 Pos3				: POSITION3;
	float4 Pos4				: POSITION4;
#endif
	float2 Tex0				: TEXCOORD0;
	float3 Normal			: NORMAL;
	float3 Tangent			: TANGENT;
};

float4 TransformPosWS(VS_INPUT In)
{
#if INSTANCE
	float4x4 g_World = {In.Pos1, In.Pos2, In.Pos3, In.Pos4};
#endif
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
#if INSTANCE
	float4x4 g_World = {In.Pos1, In.Pos2, In.Pos3, In.Pos4};
#endif
	return normalize(mul(In.Normal, (float3x3)g_World));
}

float3 TransformTangent(VS_INPUT In)
{
#if INSTANCE
	float4x4 g_World = {In.Pos1, In.Pos2, In.Pos3, In.Pos4};
#endif
	return normalize(mul(In.Tangent, (float3x3)g_World));
}

float4 TransformLightWS(VS_INPUT In)
{
	return float4(g_World[3].xyz,length(g_World[0].xyz));
}

float4 TransformColor(VS_INPUT In)
{
	return g_MeshColor;
}
