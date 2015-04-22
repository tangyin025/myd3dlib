
//--------------------------------------------------------------------------------------
// Global variables
//--------------------------------------------------------------------------------------

shared float4x4 g_World;
shared float4x4 g_View;
shared float4x4 g_ViewProj;

//--------------------------------------------------------------------------------------
// VS_INPUT
//--------------------------------------------------------------------------------------

struct VS_INPUT_SHADOW
{
	float4 Pos				: POSITION;
#if defined(VS_SKINED_DQ)
	float4 BlendWeights		: BLENDWEIGHT;
	float4 BlendIndices		: BLENDINDICES;
#endif
	float2 Tex0				: TEXCOORD0;
#if defined(VS_INSTANCE)
	float4 mat1				: POSITION1;
	float4 mat2				: POSITION2;
	float4 mat3				: POSITION3;
	float4 mat4				: POSITION4;
#endif
};

struct VS_INPUT
{
	float4 Pos				: POSITION;
#if defined(VS_SKINED_DQ)
	float4 BlendWeights		: BLENDWEIGHT;
	float4 BlendIndices		: BLENDINDICES;
#endif
	float3 Normal			: NORMAL;
	float3 Tangent			: TANGENT;
	float2 Tex0				: TEXCOORD0;
#if defined(VS_INSTANCE)
	float4 mat1				: POSITION1;
	float4 mat2				: POSITION2;
	float4 mat3				: POSITION3;
	float4 mat4				: POSITION4;
#endif
};

//--------------------------------------------------------------------------------------
// VS_OUTPUT
//--------------------------------------------------------------------------------------

struct VS_OUTPUT_SHADOW
{
	float4 Pos				: POSITION;
	float2 Tex0				: TEXCOORD0;
};

struct VS_OUTPUT
{
	float4 PosPS			: POSITION;
	float2 Tex0				: TEXCOORD0;
	float3 NormalWS			: TEXCOORD1;
	float3 TangentWS		: TEXCOORD2;
	float3 BinormalWS		: TEXCOORD3;
};

//--------------------------------------------------------------------------------------
// Texture samplers
//--------------------------------------------------------------------------------------

shared texture g_MeshTexture;              // Color texture for mesh

sampler MeshTextureSampler = 
sampler_state
{
    Texture = <g_MeshTexture>;
    MipFilter = LINEAR;
    MinFilter = LINEAR;
    MagFilter = LINEAR;
};

//--------------------------------------------------------------------------------------
// func
//--------------------------------------------------------------------------------------

void get_skinned_dual( row_major float2x4 dualquat[96],
					   float4 BlendWeights,
					   float4 BlendIndices,
					   out float2x4 dual)
{
	float2x4 m = dualquat[BlendIndices.x];
	float4 dq0 = (float1x4)m;
	dual = BlendWeights.x * m;
	m = dualquat[BlendIndices.y];
	float4 dq = (float1x4)m;
	if (dot( dq0, dq ) < 0)
		dual -= BlendWeights.y * m;
	else
		dual += BlendWeights.y * m;
	m = dualquat[BlendIndices.z];
	dq = (float1x4)m;
	if (dot( dq0, dq ) < 0)
		dual -= BlendWeights.z * m;
	else
		dual += BlendWeights.z * m;
	m = dualquat[BlendIndices.w];
	dq = (float1x4)m;
	if (dot( dq0, dq ) < 0)
		dual -= BlendWeights.w * m;
	else
		dual += BlendWeights.w * m;
	float length = sqrt(dual[0].w * dual[0].w + dual[0].x * dual[0].x + dual[0].y * dual[0].y + dual[0].z * dual[0].z);
	dual = dual / length;
}

void get_skinned_vs( row_major float2x4 dualquat[96],
					 float4 Position,
					 float4 BlendWeights,
					 float4 BlendIndices,
					 out float4 oPos)
{
	float2x4 dual;
	get_skinned_dual(dualquat, BlendWeights, BlendIndices, dual);
	oPos.xyz = Position.xyz + 2.0 * cross(dual[0].xyz, cross(dual[0].xyz, Position.xyz) + dual[0].w * Position.xyz);
	float3 translation = 2.0 * (dual[0].w * dual[1].xyz - dual[1].w * dual[0].xyz + cross(dual[0].xyz, dual[1].xyz));
	oPos.xyz += translation;
	oPos.w = 1;
}

void get_skinned_vs( row_major float2x4 dualquat[96],
					 float4 Position,
					 float3 Normal,
					 float3 Tangent,
					 float4 BlendWeights,
					 float4 BlendIndices,
					 out float4 oPos,
					 out float3 oNormal,
					 out float3 oTangent)
{
	float2x4 dual;
	get_skinned_dual(dualquat, BlendWeights, BlendIndices, dual);
	oPos.xyz = Position.xyz + 2.0 * cross(dual[0].xyz, cross(dual[0].xyz, Position.xyz) + dual[0].w * Position.xyz);
	float3 translation = 2.0 * (dual[0].w * dual[1].xyz - dual[1].w * dual[0].xyz + cross(dual[0].xyz, dual[1].xyz));
	oPos.xyz += translation;
	oPos.w = 1;
	oNormal = Normal.xyz + 2.0 * cross(dual[0].xyz, cross(dual[0].xyz, Normal.xyz) + dual[0].w * Normal.xyz);
	oTangent = Tangent.xyz + 2.0 * cross(dual[0].xyz, cross(dual[0].xyz, Tangent.xyz) + dual[0].w * Tangent.xyz);;
}
