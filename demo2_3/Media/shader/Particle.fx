
#include "CommonHeader.fx"

//--------------------------------------------------------------------------------------
// Global variables
//--------------------------------------------------------------------------------------

float3 g_CameraDir;
float3 g_CameraUp;
float3 g_CameraRight;
texture g_MeshTexture;
float2 g_AnimationColumnRow;

//--------------------------------------------------------------------------------------
// Texture samplers
//--------------------------------------------------------------------------------------

sampler MeshTextureSampler = 
sampler_state
{
    Texture = <g_MeshTexture>;
    MipFilter = LINEAR;
    MinFilter = LINEAR;
    MagFilter = LINEAR;
};

//--------------------------------------------------------------------------------------
// Vertex shader output structure
//--------------------------------------------------------------------------------------

struct VS_OUTPUT
{
    float4 Position   : POSITION;   // vertex position 
    float4 Diffuse    : COLOR0;     // vertex diffuse color (note that COLOR0 is clamped from 0..1)
    float2 TextureUV  : TEXCOORD0;  // vertex texture coords 
};

//--------------------------------------------------------------------------------------
// This shader computes standard transform and lighting
//--------------------------------------------------------------------------------------

VS_OUTPUT RenderSceneVS( float2 vTexCoord0 : TEXCOORD0, 
						 float4 vPos : POSITION0,
                         float4 vDiffuse : COLOR0,
						 float4 vTexCoord1 : TEXCOORD1,
						 float4 vTexCoord2 : TEXCOORD2 )
{
    VS_OUTPUT Output;
	float4 LocalPos = float4(rotate_angle_axis(
		g_CameraUp * lerp(vTexCoord1.y * 0.5, -vTexCoord1.y * 0.5, vTexCoord0.y) +
		g_CameraRight * lerp(-vTexCoord1.x * 0.5, vTexCoord1.x * 0.5, vTexCoord0.x), vTexCoord1.z, g_CameraDir), 0);
		
	Output.Position = mul(LocalPos + vPos, g_mWorldViewProjection);
	
	Output.Diffuse = vDiffuse;
	Output.TextureUV = (vTexCoord2.xy + vTexCoord0.xy) / g_AnimationColumnRow;
    
    return Output;    
}

//--------------------------------------------------------------------------------------
// This shader outputs the pixel's color by modulating the texture's
// color with diffuse material color
//--------------------------------------------------------------------------------------

float4 RenderScenePS( VS_OUTPUT In ) : COLOR0
{ 
    return tex2D(MeshTextureSampler, In.TextureUV) * In.Diffuse;
}

//--------------------------------------------------------------------------------------
// Renders scene 
//--------------------------------------------------------------------------------------

technique RenderScene
{
    pass P0
    {          
		CullMode = NONE;
		Lighting = FALSE;
		AlphaBlendEnable = TRUE;
		BlendOp = ADD;
		SrcBlend = SRCCOLOR;
		DestBlend = ONE;
		ZEnable = TRUE;
		ZWriteEnable = FALSE;
        VertexShader = compile vs_2_0 RenderSceneVS();
        PixelShader  = compile ps_2_0 RenderScenePS(); 
    }
}
