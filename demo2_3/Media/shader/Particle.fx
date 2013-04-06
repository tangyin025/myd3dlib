
#include "CommonHeader.fx"

//--------------------------------------------------------------------------------------
// Global variables
//--------------------------------------------------------------------------------------

shared float4 g_CameraDir;
shared float4 g_CameraUp;
texture g_MeshTexture;

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
                         float4 vDiffuse : COLOR0 )
{
    VS_OUTPUT Output;
	float4 vParticlePos = float4(lerp(-.5,.5,vTexCoord0.x), lerp(.5,-.5,vTexCoord0.y), 0, 0);
	Output.Position = mul(vParticlePos + vPos, g_mWorldViewProjection);
	
	Output.Diffuse = vDiffuse;
	Output.TextureUV = vTexCoord0;
    
    return Output;    
}

//--------------------------------------------------------------------------------------
// This shader outputs the pixel's color by modulating the texture's
// color with diffuse material color
//--------------------------------------------------------------------------------------

float4 RenderScenePS( VS_OUTPUT In ) : COLOR0
{ 
    return In.Diffuse;
}

//--------------------------------------------------------------------------------------
// Renders scene 
//--------------------------------------------------------------------------------------

technique RenderScene
{
    pass P0
    {          
		CullMode = NONE;
        VertexShader = compile vs_2_0 RenderSceneVS();
        PixelShader  = compile ps_2_0 RenderScenePS(); 
    }
}
