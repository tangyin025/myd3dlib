shared float g_Time;
shared float2 g_ScreenDim;
shared float4x4 g_World;
shared float4x4 g_View;
shared float4x4 g_ViewProj;
shared float4x4 g_InvViewProj;
shared float3 g_Eye;
shared float3 g_SkyLightDir;
shared float4x4 g_SkyLightViewProj;
shared float4 g_SkyLightColor;
shared texture g_ShadowRT;
shared texture g_NormalRT;
shared texture g_PositionRT;
shared texture g_LightRT;
shared texture g_OpaqueRT;
shared texture g_DownFilterRT;
shared texture g_MeshTexture;
shared texture g_NormalTexture;
shared texture g_SpecularTexture;
shared texture g_ReflectTexture;
shared texture g_RefractTexture;

sampler ShadowRTSampler = sampler_state
{
	Texture = <g_ShadowRT>;
	MipFilter = NONE;
	MinFilter = POINT;
	MagFilter = POINT;
};

sampler NormalRTSampler = sampler_state
{
	Texture = <g_NormalRT>;
	MipFilter = NONE;
	MinFilter = POINT;
	MagFilter = POINT;
};

sampler PositionRTSampler = sampler_state
{
	Texture = <g_PositionRT>;
	MipFilter = NONE;
	MinFilter = POINT;
	MagFilter = POINT;
};

sampler LightRTSampler = sampler_state
{
	Texture = <g_LightRT>;
	MipFilter = NONE;
	MinFilter = POINT;
	MagFilter = POINT;
};

sampler OpaqueRTSampler = sampler_state
{
	Texture = <g_OpaqueRT>;
	MipFilter = NONE;
	MinFilter = POINT;
	MagFilter = POINT;
};

sampler DownFilterRTSampler = sampler_state
{
	Texture = <g_DownFilterRT>;
	MipFilter = NONE;
	MinFilter = POINT;
	MagFilter = POINT;
};

sampler MeshTextureSampler = sampler_state
{
    Texture = <g_MeshTexture>;
    MipFilter = LINEAR;
    MinFilter = LINEAR;
    MagFilter = LINEAR;
};

sampler NormalTextureSampler = sampler_state
{
	Texture = <g_NormalTexture>;
	MipFilter = LINEAR;
	MinFilter = LINEAR;
	MagFilter = LINEAR;
};

sampler SpecularTextureSampler = sampler_state
{
	Texture = <g_SpecularTexture>;
	MipFilter = LINEAR;
	MinFilter = LINEAR;
	MagFilter = LINEAR;
};

sampler ReflectTextureSampler = sampler_state
{
	Texture = <g_ReflectTexture>;
	MipFilter = LINEAR;
	MinFilter = LINEAR;
	MagFilter = LINEAR;
};

sampler RefractTextureSampler = sampler_state
{
	Texture = <g_RefractTexture>;
	MipFilter = LINEAR;
	MinFilter = LINEAR;
	MagFilter = LINEAR;
};

float3 Reflection(float3 Normal, float3 View)
{
	return normalize(2 * dot(Normal, View) * Normal - View);
}
