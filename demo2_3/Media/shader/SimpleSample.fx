
#include "CommonHeader.fx"

//--------------------------------------------------------------------------------------
// Global variables
//--------------------------------------------------------------------------------------

#if defined(VS_SKINED_DQ)
row_major float2x4 g_dualquat[96];
#endif

//--------------------------------------------------------------------------------------
// VS
//--------------------------------------------------------------------------------------

VS_OUTPUT_SHADOW RenderShadowVS( VS_INPUT_SHADOW In )
{
	VS_OUTPUT_SHADOW Output;
#if defined(VS_INSTANCE)
	float4x4 g_World = float4x4(In.mat1,In.mat2,In.mat3,In.mat4);
#endif
#if defined(VS_SKINED_DQ)
	get_skinned_vs(g_dualquat, In.Pos, In.BlendWeights, In.BlendIndices, Output.Pos);
    Output.Pos = mul(Output.Pos, mul(g_World, g_ViewProj));
#else
    Output.Pos = mul(In.Pos, mul(g_World, g_ViewProj));
#endif
	Output.Tex0 = Output.Pos.zw;
	return Output;
}

VS_OUTPUT RenderSceneVS( VS_INPUT In )
{
    VS_OUTPUT Output;
#if defined(VS_INSTANCE)
	float4x4 g_World = float4x4(In.mat1,In.mat2,In.mat3,In.mat4);
#endif
#if defined(VS_SKINED_DQ)
	float4 Pos;
	float3 Normal, Tangent;
	get_skinned_vs(g_dualquat, In.Pos, In.Normal, In.Tangent, In.BlendWeights, In.BlendIndices, Pos, Normal, Tangent);
    Output.PosPS = mul(Pos, mul(g_World, g_ViewProj));
    Output.NormalWS = mul(Normal, (float3x3)g_World);
	Output.TangentWS = mul(Tangent, (float3x3)g_World);
#else
    Output.PosPS = mul(In.Pos, mul(g_World, g_ViewProj));
    Output.NormalWS = mul(In.Normal, (float3x3)g_World);
	Output.TangentWS = mul(In.Tangent, (float3x3)g_World);
#endif
	Output.BinormalWS = cross(Output.NormalWS, Output.TangentWS);
	Output.Tex0 = In.Tex0;
    return Output;    
}

//--------------------------------------------------------------------------------------
// PS
//--------------------------------------------------------------------------------------

float4 RenderScenePS( VS_OUTPUT In ) : COLOR0
{ 
    float4 color = tex2D(MeshTextureSampler, In.Tex0) * dot(In.NormalWS, normalize(float3(1,1,0)));
    return color;
}

float4 RenderShadowPS( VS_OUTPUT_SHADOW In ) : COLOR0
{ 
    return In.Tex0.x / In.Tex0.y;
}

//--------------------------------------------------------------------------------------
// Renders scene 
//--------------------------------------------------------------------------------------

technique RenderShadow
{
	pass p0
	{
        VertexShader = compile vs_2_0 RenderShadowVS();
        PixelShader  = compile ps_2_0 RenderShadowPS(); 
	}
}

technique RenderScene
{
    pass P0
    {          
		// FILLMODE = WIREFRAME;
		CullMode = NONE;
        VertexShader = compile vs_2_0 RenderSceneVS();
        PixelShader  = compile ps_2_0 RenderScenePS(); 
    }
}
