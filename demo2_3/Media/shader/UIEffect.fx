
#include "CommonHeader.hlsl"

texture g_MeshTexture;

sampler MeshTextureSampler = sampler_state
{
    Texture = <g_MeshTexture>;
    MipFilter = LINEAR;
    MinFilter = LINEAR;
    MagFilter = LINEAR;
    ADDRESSU = CLAMP;
    ADDRESSV = CLAMP;
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

float4 AlignUnit(float4 pos)
{
	return float4(
        ((floor((g_ScreenDim.x + pos.x / pos.w * g_ScreenDim.x) * 0.5 + 0.222222) - 0.5) * 2 - g_ScreenDim.x) / g_ScreenDim.x * pos.w,
        (g_ScreenDim.y - (floor((g_ScreenDim.y - pos.y / pos.w * g_ScreenDim.y) * 0.5 + 0.222222) - 0.5) * 2) / g_ScreenDim.y * pos.w, pos.z, pos.w);
}

VS_OUTPUT RenderSceneVS( float4 vPos : POSITION, 
                         float4 vDiffuse : COLOR0,
                         float2 vTexCoord0 : TEXCOORD0 )
{
    VS_OUTPUT Output;
    
    // Transform the position from object space to homogeneous projection space
    Output.Position = AlignUnit(mul(mul(vPos, g_World), g_ViewProj));
    
    // Calc diffuse color    
    Output.Diffuse = vDiffuse;
    
    // Just copy the texture coordinate through
    Output.TextureUV = vTexCoord0; 
    
    return Output;    
}

//--------------------------------------------------------------------------------------
// This shader outputs the pixel's color by modulating the texture's
// color with diffuse material color
//--------------------------------------------------------------------------------------

float4 RenderScenePS( VS_OUTPUT In ) : COLOR0
{ 
    // Lookup mesh texture and modulate it with diffuse
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
		SrcBlend = SRCALPHA;
		DestBlend = INVSRCALPHA;
		ZEnable = FALSE;
        VertexShader = compile vs_3_0 RenderSceneVS();
        PixelShader  = compile ps_3_0 RenderScenePS(); 
    }
}
