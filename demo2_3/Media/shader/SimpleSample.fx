
#include "CommonHeader.fx"

//--------------------------------------------------------------------------------------
// Global variables
//--------------------------------------------------------------------------------------

texture g_MeshTexture;              // Color texture for mesh
#ifdef VS_SKINED_DQ
row_major float2x4 g_dualquat[96];
#elif defined VS_SKINED_APEX
float4x3 g_BoneMatrices[60];
#endif

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
// func
//--------------------------------------------------------------------------------------

void get_skinned_dual( row_major float2x4 dualquat[96],
					   float4 BlendWeights,
					   float4 BlendIndices,
					   out float2x4 dual)
{
	float2x4 m = dualquat[BlendIndices.x];
	float4 dq0 = (float1x4)m;
	dual = BlendWeights.x * m;
	m = dualquat[BlendIndices.y];
	float4 dq = (float1x4)m;
	if (dot( dq0, dq ) < 0)
		dual -= BlendWeights.y * m;
	else
		dual += BlendWeights.y * m;
	m = dualquat[BlendIndices.z];
	dq = (float1x4)m;
	if (dot( dq0, dq ) < 0)
		dual -= BlendWeights.z * m;
	else
		dual += BlendWeights.z * m;
	m = dualquat[BlendIndices.w];
	dq = (float1x4)m;
	if (dot( dq0, dq ) < 0)
		dual -= BlendWeights.w * m;
	else
		dual += BlendWeights.w * m;
	float length = sqrt(dual[0].w * dual[0].w + dual[0].x * dual[0].x + dual[0].y * dual[0].y + dual[0].z * dual[0].z);
	dual = dual / length;
}

void get_skinned_vs( row_major float2x4 dualquat[96],
					 float4 Position,
					 float4 BlendWeights,
					 float4 BlendIndices,
					 out float4 oPos)
{
	float2x4 dual;
	get_skinned_dual(dualquat, BlendWeights, BlendIndices, dual);
	oPos.xyz = Position.xyz + 2.0 * cross(dual[0].xyz, cross(dual[0].xyz, Position.xyz) + dual[0].w * Position.xyz);
	float3 translation = 2.0 * (dual[0].w * dual[1].xyz - dual[1].w * dual[0].xyz + cross(dual[0].xyz, dual[1].xyz));
	oPos.xyz += translation;
	oPos.w = 1;
}

void get_skinned_vs( row_major float2x4 dualquat[96],
					 float4 Position,
					 float3 Normal,
					 float3 Tangent,
					 float4 BlendWeights,
					 float4 BlendIndices,
					 out float4 oPos,
					 out float3 oNormal,
					 out float3 oTangent)
{
	float2x4 dual;
	get_skinned_dual(dualquat, BlendWeights, BlendIndices, dual);
	oPos.xyz = Position.xyz + 2.0 * cross(dual[0].xyz, cross(dual[0].xyz, Position.xyz) + dual[0].w * Position.xyz);
	float3 translation = 2.0 * (dual[0].w * dual[1].xyz - dual[1].w * dual[0].xyz + cross(dual[0].xyz, dual[1].xyz));
	oPos.xyz += translation;
	oPos.w = 1;
	oNormal = Normal.xyz + 2.0 * cross(dual[0].xyz, cross(dual[0].xyz, Normal.xyz) + dual[0].w * Normal.xyz);
	oTangent = Tangent.xyz + 2.0 * cross(dual[0].xyz, cross(dual[0].xyz, Tangent.xyz) + dual[0].w * Tangent.xyz);;
}

//--------------------------------------------------------------------------------------
// VS
//--------------------------------------------------------------------------------------

VS_OUTPUT_SHADOW RenderShadowVS( VS_INPUT_SHADOW In )
{
	VS_OUTPUT_SHADOW Output;
#ifdef VS_SKINED_DQ
	get_skinned_vs(g_dualquat, In.Pos, In.BlendWeights, In.BlendIndices, Output.Pos);
    Output.Pos = mul(Output.Pos, mul(g_World, g_ViewProj));
#elif defined VS_SKINED_APEX
	Output.Pos = mul(In.Pos, mul(g_BoneMatrices[In.BlendIndices.x], mul(g_World, g_ViewProj)));
#else
    Output.Pos = mul(In.Pos, mul(g_World, g_ViewProj));
#endif
	Output.Tex0 = Output.Pos.zw;
	return Output;
}

VS_OUTPUT RenderSceneVS( VS_INPUT In )
{
    VS_OUTPUT Output;
#ifdef VS_SKINED_DQ
	get_skinned_vs(g_dualquat, In.Pos, In.Normal, In.Tangent, In.BlendWeights, In.BlendIndices, Output.Pos, Output.Normal, Output.Tangent);
    Output.Pos = mul(Output.Pos, mul(g_World, g_ViewProj));
    Output.Normal = mul(Output.Normal, (float3x3)g_World);
	Output.Tangent = mul(Output.Tangent, (float3x3)g_World);
#elif defined VS_SKINED_APEX
	Output.Pos = mul(float4(mul(In.Pos, g_BoneMatrices[In.BlendIndices.x]), 1.0), mul(g_World, g_ViewProj));
    Output.Normal = mul(In.Normal, (float3x3)mul(g_BoneMatrices[In.BlendIndices.x], g_World));
	Output.Tangent = mul(In.Tangent, (float3x3)mul(g_BoneMatrices[In.BlendIndices.x], g_World));
#else
    Output.Pos = mul(In.Pos, mul(g_World, g_ViewProj));
    Output.Normal = mul(In.Normal, (float3x3)g_World);
	Output.Tangent = mul(In.Tangent, (float3x3)g_World);
#endif
	Output.Binormal = cross(Output.Normal, Output.Tangent);
	Output.Tex0 = In.Tex0;
    return Output;    
}

//--------------------------------------------------------------------------------------
// PS
//--------------------------------------------------------------------------------------

float4 RenderScenePS( VS_OUTPUT In ) : COLOR0
{ 
    float4 color = tex2D(MeshTextureSampler, In.Tex0);
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
        VertexShader = compile vs_2_0 RenderSceneVS();
        PixelShader  = compile ps_2_0 RenderScenePS(); 
    }
}
