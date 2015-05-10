
shared float4x4 g_World;
shared float4x4 g_View;
shared float4x4 g_ViewProj;
shared float4x4 g_InvViewProj;
shared float2 g_ScreenDim;
shared float3 g_SkyLightDir;
shared float4x4 g_SkyLightViewProj;
shared texture g_ShadowRT;
shared texture g_NormalRT;
shared texture g_DiffuseRT;
shared texture g_MeshTexture;
shared texture g_NormalTexture;
shared texture g_SpecularTexture;

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

sampler DiffuseRTSampler = sampler_state
{
	Texture = <g_DiffuseRT>;
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
