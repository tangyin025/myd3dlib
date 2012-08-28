
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

struct VS_INPUT
{
    float4  Pos             : POSITION;
    float4  BlendWeights    : BLENDWEIGHT;
    float4  BlendIndices    : BlendIndices;
    float3  Normal          : NORMAL;
    float3  Tex0            : TEXCOORD0;
};

struct VS_OUTPUT
{
    float4 Position   : POSITION;   // vertex position 
    float4 Diffuse    : COLOR0;     // vertex diffuse color (note that COLOR0 is clamped from 0..1)
    float2 TextureUV  : TEXCOORD0;  // vertex texture coords 
};

//--------------------------------------------------------------------------------------
// This shader computes standard transform and lighting
//--------------------------------------------------------------------------------------

VS_OUTPUT RenderSceneVS( VS_INPUT i )
{
    VS_OUTPUT Output;
    float3 vNormalWorldSpace;
	
    float2x4 dual = (float2x4)0;
    float2x4 m = g_dualquat[i.BlendIndices.x];
    float4 dq0 = (float1x4)m;
	dual = i.BlendWeights.x * m;
	
    m = g_dualquat[i.BlendIndices.y];
    float4 dq = (float1x4)m;
	if (dot( dq0, dq ) < 0)
		dual -= i.BlendWeights.y * m;
    else
		dual += i.BlendWeights.y * m;
		
	m = g_dualquat[i.BlendIndices.z];
    dq = (float1x4)m;
    if (dot( dq0, dq ) < 0)
		dual -= i.BlendWeights.z * m;
	else
		dual += i.BlendWeights.z * m;
		
	m = g_dualquat[i.BlendIndices.w];
    dq = (float1x4)m;
    if (dot( dq0, dq ) < 0)
		dual -= i.BlendWeights.w * m;
	else
		dual += i.BlendWeights.w * m;
		
	// fast dqs 
    float length = sqrt(dual[0].w * dual[0].w + dual[0].x * dual[0].x + dual[0].y * dual[0].y + dual[0].z * dual[0].z);
    dual = dual / length;
    float3 position = i.Pos.xyz + 2.0 * cross(dual[0].xyz, cross(dual[0].xyz, i.Pos.xyz) + dual[0].w * i.Pos.xyz);
    float3 translation = 2.0 * (dual[0].w * dual[1].xyz - dual[1].w * dual[0].xyz + cross(dual[0].xyz, dual[1].xyz));
    position += translation;
	
    float3 normal = i.Normal.xyz + 2.0 * cross(dual[0].xyz, cross(dual[0].xyz,i.Normal.xyz) + dual[0].w * i.Normal.xyz);
    float4 vAnimatedPos = float4(position, 1);
    float4 vAnimatedNormal = float4(normal, 0);
	
    // Transform the position from object space to homogeneous projection space
	Output.Position = mul(vAnimatedPos, g_mWorldViewProjection);
    
    // Transform the Normal from object space to world space    
	vNormalWorldSpace = normalize(mul(vAnimatedNormal, (float3x3)g_mWorld));
	
    // Calc diffuse color    
    Output.Diffuse.rgb = g_MaterialDiffuseColor * g_LightDiffuse * max(0, dot(vNormalWorldSpace, g_LightDir)) + 
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
