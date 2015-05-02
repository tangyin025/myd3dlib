
row_major float2x4 g_dualquat[96];

struct VS_INPUT
{
	float4 Pos				: POSITION;
	float2 Tex0				: TEXCOORD0;
	float3 Normal			: NORMAL;
	float4 BlendWeights		: BLENDWEIGHT;
	float4 BlendIndices		: BLENDINDICES;
};

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

void GetSkinnedPos( VS_INPUT In,
					out float4 oPos)
{
	float2x4 dual;
	GetSkinnedDual(In, dual);
	oPos.xyz = In.Pos.xyz + 2.0 * cross(dual[0].xyz, cross(dual[0].xyz, In.Pos.xyz) + dual[0].w * In.Pos.xyz);
	float3 translation = 2.0 * (dual[0].w * dual[1].xyz - dual[1].w * dual[0].xyz + cross(dual[0].xyz, dual[1].xyz));
	oPos.xyz += translation;
	oPos.w = 1;
}

void GetSkinnedNormal( VS_INPUT In,
					   out float3 oNormal)
{
	float2x4 dual;
	GetSkinnedDual(In, dual);
	oNormal = In.Normal.xyz + 2.0 * cross(dual[0].xyz, cross(dual[0].xyz, In.Normal.xyz) + dual[0].w * In.Normal.xyz);
}

float4 TransformPosWS(VS_INPUT In)
{
	float4 pos;
	GetSkinnedPos(In, pos);
    return mul(pos, g_World);
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
	float3 normal;
	GetSkinnedNormal(In, normal);
	return normalize(mul(normal, (float3x3)g_World));
}

float4 TransformLight(VS_INPUT In)
{
	return float4(0,0,0,0);
}
