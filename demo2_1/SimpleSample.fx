
// ------------------------------------------------------------------------------------------
// Global variables
// ------------------------------------------------------------------------------------------

float		g_fTime;
float4x4	g_mWorld;
float4x4	g_mWorldViewProjection;

float4		g_MaterialAmbientColor;
float4		g_MaterialDiffuseColor;
float3		g_LightDir;
float4		g_LightDiffuse;
texture		g_MeshTexture;

// ------------------------------------------------------------------------------------------
// Texture samplers
// ------------------------------------------------------------------------------------------

sampler MeshTextureSampler =
sampler_state
{
	Texture		= <g_MeshTexture>;
	MinFilter	= LINEAR;
	MagFilter	= LINEAR;
	MipFilter	= NONE;
};


// ------------------------------------------------------------------------------------------
// Vertex shader output structure
// ------------------------------------------------------------------------------------------

struct VS_OUTPUT
{
	float4 Position		: POSITION;
	float4 Diffuse		: COLOR0;
	float2 TextureUV	: TEXCOORD0;
};

// ------------------------------------------------------------------------------------------
// This shader computes standard transform and lighting
// ------------------------------------------------------------------------------------------

VS_OUTPUT RenderSceneVS(float4 vPos			: POSITION,
						float3 vNormal		: NORMAL,
						float2 vTexCoord0	: TEXCOORD0)
{
	VS_OUTPUT Output;

	// 将顶点坐标直接变换到相机投影空间
	Output.Position = mul(vPos, g_mWorldViewProjection);

	// 计算世界坐标中的法线
	float3 vNormalWorldSpace = normalize(mul(vNormal, (float3x3)g_mWorld));

	// 通过世界坐标中的法线及光源信息计算顶点颜色
	Output.Diffuse.rgb =
		g_MaterialDiffuseColor * g_LightDiffuse * max(0, -dot(vNormalWorldSpace, g_LightDir)) + g_MaterialAmbientColor;
	Output.Diffuse.a = 1.0f;

	// 直接复制贴图坐标
	Output.TextureUV = vTexCoord0;

	return Output;
}

// ------------------------------------------------------------------------------------------
// Pixel shader output structure
// ------------------------------------------------------------------------------------------

struct PS_OUTPUT
{
	float4 RGBColor		: COLOR0;
};

// ------------------------------------------------------------------------------------------
// This shader outputs the pixel's color by modulating the texture's color with diffuse material color
// ------------------------------------------------------------------------------------------

PS_OUTPUT RenderScenePS(VS_OUTPUT In)
{
	PS_OUTPUT Output;

	// 根据贴图坐标进行采样
	Output.RGBColor = tex2D(MeshTextureSampler, In.TextureUV) * In.Diffuse;

	return Output;
}

// ------------------------------------------------------------------------------------------
// Renders scene
// ------------------------------------------------------------------------------------------

technique RenderScene
{
	pass P0
	{
		VertexShader =
			compile vs_2_0 RenderSceneVS();

		PixelShader =
			compile ps_2_0 RenderScenePS();
	}
}
