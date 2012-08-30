
#include "CommonHeader.fx"

//--------------------------------------------------------------------------------------
// Global variables
//--------------------------------------------------------------------------------------

float4 g_MaterialAmbientColor;
float4 g_MaterialDiffuseColor;
float3 g_LightDir;
float4 g_LightDiffuse;
texture g_MeshTexture;

//--------------------------------------------------------------------------------------
// Texture samplers
//--------------------------------------------------------------------------------------

sampler MeshTextureSampler = 
sampler_state
{
    Texture = <g_MeshTexture>;
    MinFilter = LINEAR;
    MagFilter = LINEAR;
    MipFilter = LINEAR;
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

VS_OUTPUT RenderSceneVS( SKINED_VS_INPUT i )
{
	float4 pos;
	float3 normal;
	get_skined_vs(i, pos, normal);
	
    VS_OUTPUT Output;
	Output.Position = mul(pos, g_mWorldViewProjection);
	
	float3 NormalWS = mul(normal, g_mWorld);
	
    // Calc diffuse color    
    Output.Diffuse.rgb = g_MaterialDiffuseColor * g_LightDiffuse * max(0, dot(NormalWS, g_LightDir)) + 
                         g_MaterialAmbientColor;
    Output.Diffuse.a = 1.0f;
    
    // Just copy the texture coordinate through
    Output.TextureUV = i.Tex0;
	
	return Output;    
}

//--------------------------------------------------------------------------------------
// Pixel shader output structure
//--------------------------------------------------------------------------------------

struct PS_OUTPUT
{
    float4 RGBColor : COLOR0;  // Pixel color    
};

//--------------------------------------------------------------------------------------
// This shader outputs the pixel's color by modulating the texture's
// color with diffuse material color
//--------------------------------------------------------------------------------------

PS_OUTPUT RenderScenePS( VS_OUTPUT In ) 
{ 
    PS_OUTPUT Output;

    // Lookup mesh texture and modulate it with diffuse
    Output.RGBColor = tex2D(MeshTextureSampler, In.TextureUV) * In.Diffuse;

    return Output;
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
