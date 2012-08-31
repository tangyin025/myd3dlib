
#include "CommonHeader.fx"

//--------------------------------------------------------------------------------------
// Global variables
//--------------------------------------------------------------------------------------

float4 g_MaterialAmbientColor;
float4 g_MaterialDiffuseColor;
texture g_MeshTexture;

texture g_txCubeMap;
texture g_txNormalMap;
texture g_txSpecularMap;

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

samplerCUBE g_samCubeMap =
sampler_state
{
	Texture = <g_txCubeMap>;
    MipFilter = LINEAR;
    MinFilter = LINEAR;
    MagFilter = LINEAR;
};

sampler g_samNormalMap =
sampler_state
{
	Texture = <g_txNormalMap>;
    MipFilter = LINEAR;
    MinFilter = LINEAR;
    MagFilter = LINEAR;
};

sampler g_samSpecularMap =
sampler_state
{
	Texture = <g_txSpecularMap>;
    MipFilter = LINEAR;
    MinFilter = LINEAR;
    MagFilter = LINEAR;
};

//--------------------------------------------------------------------------------------
// Vertex shader output structure
//--------------------------------------------------------------------------------------

struct VS_OUTPUT
{
    float4 Position   	: POSITION;   // vertex position 
    float4 Diffuse    	: COLOR0;     // vertex diffuse color (note that COLOR0 is clamped from 0..1)
    float2 TextureUV  	: TEXCOORD0;  // vertex texture coords 
	float3 LightTS	  	: TEXCOORD1;
	float3 ViewTS		: TEXCOORD2;
};

//--------------------------------------------------------------------------------------
// This shader computes standard transform and lighting
//--------------------------------------------------------------------------------------

VS_OUTPUT RenderSceneVS( SKINED_VS_INPUT i )
{
	float4 vPos;
	float3 vNormal;
	float3 vTangent;
	get_skined_vsnormal(i, vPos, vNormal, vTangent);
	
    VS_OUTPUT Output;
    float3 vNormalWorldSpace;
    
    // Transform the position from object space to homogeneous projection space
    Output.Position = mul(vPos, g_mWorldViewProjection);
    
    // Transform the normal from object space to world space    
    vNormalWorldSpace = normalize(mul(vNormal, (float3x3)g_mWorld)); // normal (world space)

    // Calc diffuse color    
    Output.Diffuse.rgb = g_MaterialDiffuseColor * g_LightDiffuse * max(0,dot(vNormalWorldSpace, g_LightDir)) + 
                         g_MaterialAmbientColor;   
    Output.Diffuse.a = 1.0f; 
    
    // Just copy the texture coordinate through
    Output.TextureUV = i.Tex0; 
	
	float3 vTangentWS = normalize(mul(vTangent, (float3x3)g_mWorld));
	float3 vBinormalWS = cross(vNormalWorldSpace, vTangentWS);
	float3x3 mWorldToTangent = float3x3(vTangentWS, vBinormalWS, vNormalWorldSpace);
	
	Output.LightTS = mul(mWorldToTangent, g_LightDir);
	Output.ViewTS = mul(mWorldToTangent, normalize(g_EyePos - mul(g_mWorld, vPos)));
    
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

    float3 vNormalTS = tex2D(g_samNormalMap, In.TextureUV) * 2 - 1;
    
    float4 cBaseColor = tex2D(MeshTextureSampler, In.TextureUV);
    
    float3 vLightTSAdj = float3(In.LightTS.x, -In.LightTS.y, In.LightTS.z);
    
    float4 cDiffuse = saturate(dot(vNormalTS, vLightTSAdj)) * g_MaterialDiffuseColor + g_MaterialAmbientColor;
    
    float3 vReflectionTS = normalize(2 * dot(In.ViewTS, vNormalTS) * vNormalTS - In.ViewTS);
	
    float fRdotL = saturate(dot(vReflectionTS, vLightTSAdj));
    
    float4 cSpecular = saturate(pow(fRdotL, 8)) * 1.5 * tex2D(g_samSpecularMap, In.TextureUV);
    
    // Lookup mesh texture and modulate it with diffuse
    Output.RGBColor = cDiffuse * cBaseColor + cSpecular;

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
