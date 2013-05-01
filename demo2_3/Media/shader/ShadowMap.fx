
#include "CommonHeader.fx"

//--------------------------------------------------------------------------------------
// RenderShadowVS
//--------------------------------------------------------------------------------------

void RenderShadowVS(float4 Pos : POSITION,
					float3 Normal : NORMAL,
					out float4 oPos : POSITION,
					out float2 Depth : TEXCOORD0)
{
	oPos = mul(Pos, mul(g_World, g_ViewProj));
	
	Depth.xy = oPos.zw;
}

void RenderSkinedShadowVS(SKINED_VS_INPUT i,
						  out float4 oPos : POSITION,
						  out float2 Depth : TEXCOORD0)
{
	float4 Pos;
	float3 Normal;
	get_skined_vs(i, Pos);
	
	oPos = mul(Pos, mul(g_World, g_ViewProj));
	
	Depth.xy = oPos.zw;
}

//--------------------------------------------------------------------------------------
// RenderShadowPS
//--------------------------------------------------------------------------------------
void RenderShadowPS(float2 Depth : TEXCOORD0,
					out float4 Color : COLOR)
{
	Color = Depth.x / Depth.y;
}

//--------------------------------------------------------------------------------------
// Technique
//--------------------------------------------------------------------------------------

technique RenderShadow
{
	pass P0
	{
		VertexShader = compile vs_2_0 RenderShadowVS();
		PixelShader  = compile ps_2_0 RenderShadowPS();
	}
}

technique RenderSkinedShadow
{
	pass P0
	{
		VertexShader = compile vs_2_0 RenderSkinedShadowVS();
		PixelShader  = compile ps_2_0 RenderShadowPS();
	}
}
