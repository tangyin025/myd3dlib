//--------------------------------------------------------------------------------------
// File: SimpleSample.fx
//
// The effect file for the SimpleSample sample.  
// 
// Copyright (c) Microsoft Corporation. All rights reserved.
//--------------------------------------------------------------------------------------

#include "CommonHeader.fx"

//--------------------------------------------------------------------------------------
// Global variables
//--------------------------------------------------------------------------------------
float4 g_MaterialAmbientColor;      // Material's ambient color
float4 g_MaterialDiffuseColor;      // Material's diffuse color
float3 g_LightDir;                  // Light's direction in world space
float4 g_LightDiffuse;              // Light's diffuse color
texture g_MeshTexture;              // Color texture for mesh
#ifdef VS_SKINED_DQ
row_major float2x4 g_dualquat[96];
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
// get_skined_vs
//--------------------------------------------------------------------------------------
#ifdef VS_SKINED_DQ
void get_skined_vs(VS_INPUT In, out float4 Pos, out float3 Normal, out float3 Tangent)
{
	float2x4 m = g_dualquat[In.BlendIndices.x];
	float4 dq0 = (float1x4)m;
	float2x4 dual = In.BlendWeights.x * m;
	
	m = g_dualquat[In.BlendIndices.y];
	float4 dq = (float1x4)m;
	if (dot( dq0, dq ) < 0)
		dual -= In.BlendWeights.y * m;
	else
		dual += In.BlendWeights.y * m;
		
	m = g_dualquat[In.BlendIndices.z];
	dq = (float1x4)m;
	if (dot( dq0, dq ) < 0)
		dual -= In.BlendWeights.z * m;
	else
		dual += In.BlendWeights.z * m;
		
	m = g_dualquat[In.BlendIndices.w];
	dq = (float1x4)m;
	if (dot( dq0, dq ) < 0)
		dual -= In.BlendWeights.w * m;
	else
		dual += In.BlendWeights.w * m;
		
	float length = sqrt(dual[0].w * dual[0].w + dual[0].x * dual[0].x + dual[0].y * dual[0].y + dual[0].z * dual[0].z);
	dual = dual / length;
	
	float3 position = In.Pos.xyz + 2.0 * cross(dual[0].xyz, cross(dual[0].xyz, In.Pos.xyz) + dual[0].w * In.Pos.xyz);
	float3 translation = 2.0 * (dual[0].w * dual[1].xyz - dual[1].w * dual[0].xyz + cross(dual[0].xyz, dual[1].xyz));
	position += translation;
	Pos = float4(position, 1);
	
	Normal = In.Normal.xyz + 2.0 * cross(dual[0].xyz, cross(dual[0].xyz, In.Normal.xyz) + dual[0].w * In.Normal.xyz);
	Tangent = In.Tangent.xyz + 2.0 * cross(dual[0].xyz, cross(dual[0].xyz, In.Tangent.xyz) + dual[0].w * In.Tangent.xyz);;
}
#endif

//--------------------------------------------------------------------------------------
// This shader computes standard transform and lighting
//--------------------------------------------------------------------------------------
VS_OUTPUT RenderSceneVS( VS_INPUT In )
{
    VS_OUTPUT Output;
#ifdef VS_SKINED_DQ
	get_skined_vs(In, Output.Pos, Output.Normal, Output.Tangent);
    Output.Pos = mul(Output.Pos, mul(g_World, g_ViewProj));
    Output.Normal = mul(Output.Normal, (float3x3)g_World);
	Output.Tangent = mul(Output.Tangent, (float3x3)g_World);
#else
    Output.Pos = mul(In.Pos, mul(g_World, g_ViewProj));
    Output.Normal = mul(In.Normal, (float3x3)g_World);
	Output.Tangent = mul(In.Tangent, (float3x3)g_World);
#endif
	Output.Binormal = cross(Output.Normal, Output.Tangent);
	Output.View = 1.0f;
	Output.Tex0 = In.Tex0;
    
    return Output;    
}


//--------------------------------------------------------------------------------------
// This shader outputs the pixel's color by modulating the texture's
// color with diffuse material color
//--------------------------------------------------------------------------------------
float4 RenderScenePS( VS_OUTPUT In ) : COLOR0
{ 
    // Lookup mesh texture and modulate it with diffuse
    float4 color = tex2D(MeshTextureSampler, In.Tex0);

    return color;
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
