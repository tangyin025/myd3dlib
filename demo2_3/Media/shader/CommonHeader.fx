
//--------------------------------------------------------------------------------------
// Global variables
//--------------------------------------------------------------------------------------

#if defined(VS_INSTANCE)
#else
shared float4x4 g_World;
#endif
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

texture g_MeshTexture;              // Color texture for mesh

sampler MeshTextureSampler = 
sampler_state
{
    Texture = <g_MeshTexture>;
    MipFilter = LINEAR;
    MinFilter = LINEAR;
    MagFilter = LINEAR;
};
