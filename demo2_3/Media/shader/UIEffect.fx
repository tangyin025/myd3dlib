
#include "CommonHeader.fx"

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
// align_ui_unit
//--------------------------------------------------------------------------------------

float4 align_ui_unit(float4 pos, float2 ScreenDim)
{
	return float4(
		((floor((ScreenDim.x + pos.x / pos.w * ScreenDim.x) * 0.5 + 0.222222) - 0.5) * 2 - ScreenDim.x) / ScreenDim.x * pos.w,
		(ScreenDim.y - (floor((ScreenDim.y - pos.y / pos.w * ScreenDim.y) * 0.5 + 0.222222) - 0.5) * 2) / ScreenDim.y * pos.w,
		pos.z,
		pos.w);
}

//--------------------------------------------------------------------------------------
// This shader computes standard transform and lighting
//--------------------------------------------------------------------------------------

VS_OUTPUT RenderSceneVS( float4 vPos : POSITION, 
                         float4 vDiffuse : COLOR0,
                         float2 vTexCoord0 : TEXCOORD0 )
{
    VS_OUTPUT Output;
    
    // Transform the position from object space to homogeneous projection space
    Output.Position = align_ui_unit(mul(vPos, mul(g_World, g_ViewProj)), g_ScreenDim);
    
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
		SrcBlend = SRCALPHA;
		DestBlend = INVSRCALPHA;
		ZEnable = FALSE;
        VertexShader = compile vs_3_0 RenderSceneVS();
        PixelShader  = compile ps_3_0 RenderScenePS(); 
    }
}
