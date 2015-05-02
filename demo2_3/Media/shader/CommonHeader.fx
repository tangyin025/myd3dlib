
shared float4x4 g_World;
shared float4x4 g_View;
shared float4x4 g_ViewProj;
shared float4x4 g_InvViewProj;
shared float3 g_SkyLightDir;
shared float4x4 g_SkyLightViewProj;
shared texture g_MeshTexture;
shared texture g_ShadowTexture;
shared texture g_NormalTexture;
shared texture g_DiffuseTexture;

sampler MeshTextureSampler = sampler_state
{
    Texture = <g_MeshTexture>;
    MipFilter = LINEAR;
    MinFilter = LINEAR;
    MagFilter = LINEAR;
};

sampler ShadowTextureSampler = sampler_state
{
	Texture = <g_ShadowTexture>;
	MipFilter = NONE;
	MinFilter = POINT;
	MagFilter = POINT;
};

sampler NormalTextureSampler = sampler_state
{
	Texture = <g_NormalTexture>;
	MipFilter = NONE;
	MinFilter = POINT;
	MagFilter = POINT;
};

sampler DiffuseTextureSampler = sampler_state
{
	Texture = <g_DiffuseTexture>;
	MipFilter = NONE;
	MinFilter = POINT;
	MagFilter = POINT;
};
