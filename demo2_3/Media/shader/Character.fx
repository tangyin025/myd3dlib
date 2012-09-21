
#include "CommonHeader.fx"

//--------------------------------------------------------------------------------------
// Global variables
//--------------------------------------------------------------------------------------

#define SHADOW_MAP_SIZE 512
#define SHADOW_EPSILON 0.00010f

float4 g_MaterialAmbientColor;
float4 g_MaterialDiffuseColor;
texture g_MeshTexture;
texture g_NormalTexture;
texture g_SpecularTexture;
texture g_CubeTexture;

float FresExp = 3.0;
float ReflStrength = 1.4;

//--------------------------------------------------------------------------------------
// Texture samplers
//--------------------------------------------------------------------------------------

sampler ShadowTextureSampler =
sampler_state
{
	Texture = <g_ShadowTexture>;
	MipFilter = LINEAR;
	MinFilter = LINEAR;
	MagFilter = LINEAR;
};

sampler MeshTextureSampler = 
sampler_state
{
	Texture = <g_MeshTexture>;
	MinFilter = LINEAR;
	MagFilter = LINEAR;
	MipFilter = LINEAR;
};

sampler NormalTextureSampler =
sampler_state
{
	Texture = <g_NormalTexture>;
	MipFilter = LINEAR;
	MinFilter = LINEAR;
	MagFilter = LINEAR;
};

sampler SpecularTextureSampler =
sampler_state
{
	Texture = <g_SpecularTexture>;
	MipFilter = LINEAR;
	MinFilter = LINEAR;
	MagFilter = LINEAR;
};

samplerCUBE CubeTextureSampler =
sampler_state
{
	Texture = <g_CubeTexture>;
	MipFilter = LINEAR;
	MinFilter = LINEAR;
	MagFilter = LINEAR;
};

//--------------------------------------------------------------------------------------
// Vertex shader output structure
//--------------------------------------------------------------------------------------

struct VS_OUTPUT
{
	float4 Position		: POSITION;
	float2 TextureUV	: TEXCOORD0;
	float3 NormalWS		: TEXCOORD1;
	float3 TangentWS	: TEXCOORD2;
	float3 BinormalWS	: TEXCOORD3;
	float3 ViewWS		: TEXCOORD4;
	float4 PosLight		: TEXCOORD5;
};

//--------------------------------------------------------------------------------------
// This shader computes standard transform and lighting
//--------------------------------------------------------------------------------------

VS_OUTPUT RenderSceneVS( SKINED_VS_INPUT i )
{
	float4 vPos;
	float3 vNormal;
	float3 vTangent;
	get_skined_vsnormal(i, vPos, vNormal, vTangent);
	
	float3 vNormalWS = normalize(mul(vNormal, (float3x3)g_mWorld));
	float3 vTangentWS = normalize(mul(vTangent, (float3x3)g_mWorld));
	float3 vBinormalWS = cross(vNormalWS, vTangentWS);
	
	VS_OUTPUT Output;
	Output.Position = mul(vPos, g_mWorldViewProjection);
	Output.TextureUV = i.Tex0; 
	Output.NormalWS = vNormalWS;
	Output.TangentWS = vTangentWS;
	Output.BinormalWS = vBinormalWS;
	Output.ViewWS = g_EyePos - mul(vPos, g_mWorld);
	Output.PosLight = mul(vPos, mul(g_mWorld, g_mLightViewProjection));
	
	return Output;
}

//--------------------------------------------------------------------------------------
// This shader outputs the pixel's color by modulating the texture's
// color with diffuse material color
//--------------------------------------------------------------------------------------

float get_ligthAmount(float4 PosLight)
{
	float2 ShadowTexC = PosLight.xy / PosLight.w * 0.5 + 0.5;
	ShadowTexC.y = 1.0 - ShadowTexC.y;
	
	float LightAmount = 0;
	float x, y;
	for(x = -0.5; x <= 0.5; x += 1.0)
		for(y = -0.5; y <= 0.5; y+= 1.0)
			LightAmount += tex2D(ShadowTextureSampler, ShadowTexC + float2(x, y) / SHADOW_MAP_SIZE) + SHADOW_EPSILON < PosLight.z / PosLight.w ? 0.0f : 1.0f;
			
	return LightAmount / 4;
}

float4 RenderScenePS( VS_OUTPUT In ) : COLOR0
{ 
	float3x3 mT2W = float3x3(normalize(In.TangentWS), normalize(In.BinormalWS), normalize(In.NormalWS));
	
	float3 vViewWS = normalize(In.ViewWS);
	
	float3 vNormalTS = tex2D(NormalTextureSampler, In.TextureUV) * 2 - 1;
	
	float3 vNormalWS = mul(vNormalTS, mT2W);
	
	float3 vReflectionWS = get_reflection(vNormalWS, vViewWS);
	
	float4 cSpecular = texCUBE(CubeTextureSampler, vReflectionWS) * tex2D(SpecularTextureSampler, In.TextureUV);
	
	float4 cDiffuse = saturate(dot(vNormalWS, g_LightDir)) * g_MaterialDiffuseColor * get_ligthAmount(In.PosLight);
	
	float4 cAmbient = g_MaterialAmbientColor + get_fresnel(vNormalWS, vViewWS, FresExp, ReflStrength);
	
	return (cDiffuse + cAmbient) * tex2D(MeshTextureSampler, In.TextureUV) + cSpecular;
}

//--------------------------------------------------------------------------------------
// Renders scene 
//--------------------------------------------------------------------------------------

technique RenderScene
{
	pass P0
	{
		VertexShader = compile vs_2_0 RenderSceneVS();
		PixelShader  = compile ps_2_0 RenderScenePS(); 
	}
}
