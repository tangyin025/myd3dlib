
//--------------------------------------------------------------------------------------
// Global variables
//--------------------------------------------------------------------------------------

shared float4x4 g_World;
shared float4x4 g_ViewProj;

//--------------------------------------------------------------------------------------
// VS_INPUT
//--------------------------------------------------------------------------------------

struct VS_INPUT_SHADOW
{
	float4 Pos				: POSITION;
#ifdef VS_SKINED_DQ
	float4 BlendWeights		: BLENDWEIGHT;
	float4 BlendIndices		: BlendIndices;
#endif
	float2 Tex0				: TEXCOORD0;
};

struct VS_INPUT
{
	float4 Pos				: POSITION;
#ifdef VS_SKINED_DQ
	float4 BlendWeights		: BLENDWEIGHT;
	float4 BlendIndices		: BlendIndices;
#endif
	float3 Normal			: NORMAL;
	float3 Tangent			: TANGENT;
	float2 Tex0				: TEXCOORD0;
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
	float4 Pos				: POSITION;
	float2 Tex0				: TEXCOORD0;
	float3 Normal			: TEXCOORD1;
	float3 Tangent			: TEXCOORD2;
	float3 Binormal			: TEXCOORD3;
};
