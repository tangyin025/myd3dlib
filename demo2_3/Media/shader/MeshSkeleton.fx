
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
	float4 BlendWeights		: BLENDWEIGHT;
	float4 BlendIndices		: BLENDINDICES;
};

row_major float2x4 g_dualquat[96];

void GetSkinnedDual( VS_INPUT In,
					 out float2x4 dual)
{
	float2x4 m = g_dualquat[In.BlendIndices.x];
	float4 dq0 = (float1x4)m;
	dual = In.BlendWeights.x * m;
	m = g_dualquat[In.BlendIndices.y];
	float4 dq = (float1x4)m;
	if (dot( dq0, dq ) < 0)
		dual -= In.BlendWeights.y * m;
	else
		dual += In.BlendWeights.y * m;
	m = g_dualquat[In.BlendIndices.z];
	dq = (float1x4)m;
	if (dot( dq0, dq ) < 0)
		dual -= In.BlendWeights.z * m;
	else
		dual += In.BlendWeights.z * m;
	m = g_dualquat[In.BlendIndices.w];
	dq = (float1x4)m;
	if (dot( dq0, dq ) < 0)
		dual -= In.BlendWeights.w * m;
	else
		dual += In.BlendWeights.w * m;
	float length = sqrt(dual[0].w * dual[0].w + dual[0].x * dual[0].x + dual[0].y * dual[0].y + dual[0].z * dual[0].z);
	dual = dual / length;
}

float4 TransformPosWS(VS_INPUT In)
{
#if INSTANCE
	float4x4 g_World = {In.Pos1, In.Pos2, In.Pos3, In.Pos4};
#endif
	float4 pos;
	float2x4 dual;
	GetSkinnedDual(In, dual);
	pos.xyz = In.Pos.xyz + 2.0 * cross(dual[0].xyz, cross(dual[0].xyz, In.Pos.xyz) + dual[0].w * In.Pos.xyz);
	float3 translation = 2.0 * (dual[0].w * dual[1].xyz - dual[1].w * dual[0].xyz + cross(dual[0].xyz, dual[1].xyz));
	pos.xyz += translation;
	pos.w = 1;
    return mul(pos, g_World);
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
#if INSTANCE
	float4x4 g_World = {In.Pos1, In.Pos2, In.Pos3, In.Pos4};
#endif
	float2x4 dual;
	GetSkinnedDual(In, dual);
	float3 normal = In.Normal.xyz + 2.0 * cross(dual[0].xyz, cross(dual[0].xyz, In.Normal.xyz) + dual[0].w * In.Normal.xyz);
	return normalize(mul(normal, (float3x3)g_World));
}

float3 TransformTangent(VS_INPUT In)
{
#if INSTANCE
	float4x4 g_World = {In.Pos1, In.Pos2, In.Pos3, In.Pos4};
#endif
	float2x4 dual;
	GetSkinnedDual(In, dual);
	float3 Tangent = In.Tangent.xyz + 2.0 * cross(dual[0].xyz, cross(dual[0].xyz, In.Tangent.xyz) + dual[0].w * In.Tangent.xyz);;
	return normalize(mul(Tangent, (float3x3)g_World));
}

float4 TransformLightWS(VS_INPUT In)
{
	return float4(0,0,0,0);
}

float4 TransformColor(VS_INPUT In)
{
	return g_MeshColor;
}
