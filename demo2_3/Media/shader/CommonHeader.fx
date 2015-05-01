
shared float4x4 g_World;
shared float4x4 g_View;
shared float4x4 g_ViewProj;
shared float3 g_SkyLightDir;
shared float4x4 g_SkyLightViewProj;
shared texture g_MeshTexture;
shared texture g_ShadowTexture;

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
	MipFilter = LINEAR;
	MinFilter = LINEAR;
	MagFilter = LINEAR;
};
