//--------------------------------------------------------------------------------------
// File: SimpleSample.fx
//
// The effect file for the SimpleSample sample.  
// 
// Copyright (c) Microsoft Corporation. All rights reserved.
//--------------------------------------------------------------------------------------

#include "CommonHeader.fx"

static const int g_cKernelSize = 13;

float4 FocalPlane = float4( 0.0f, 0.0f, 0.2f, 1.0f );

float2 PixelKernel[g_cKernelSize] =
{
    { -6, 0 },
    { -5, 0 },
    { -4, 0 },
    { -3, 0 },
    { -2, 0 },
    { -1, 0 },
    {  0, 0 },
    {  1, 0 },
    {  2, 0 },
    {  3, 0 },
    {  4, 0 },
    {  5, 0 },
    {  6, 0 },
};

float2 PixelKernel2[g_cKernelSize] =
{
    { 0, -6 },
    { 0, -5 },
    { 0, -4 },
    { 0, -3 },
    { 0, -2 },
    { 0, -1 },
    { 0,  0 },
    { 0,  1 },
    { 0,  2 },
    { 0,  3 },
    { 0,  4 },
    { 0,  5 },
    { 0,  6 },
};

static const float BlurWeights[g_cKernelSize] = 
{
    0.002216,
    0.008764,
    0.026995,
    0.064759,
    0.120985,
    0.176033,
    0.199471,
    0.176033,
    0.120985,
    0.064759,
    0.026995,
    0.008764,
    0.002216,
};

//--------------------------------------------------------------------------------------
// Vertex shader output structure
//--------------------------------------------------------------------------------------
struct VS_OUTPUT
{
    float4 Position   : POSITION;   // vertex position 
    float2 TextureUV  : TEXCOORD0;  // vertex texture coords 
};


//--------------------------------------------------------------------------------------
// This shader computes standard transform and lighting
//--------------------------------------------------------------------------------------
VS_OUTPUT RenderSceneVS( float4 vPos : POSITION, 
                         float2 vTexCoord0 : TEXCOORD0 )
{
    VS_OUTPUT Output;
    
    // Transform the position from object space to homogeneous projection space
    Output.Position = mul(vPos, g_World);
    
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
    return tex2D(DownFilterRTSampler, In.TextureUV);
}

float4 PS1( VS_OUTPUT In ) : COLOR0
{ 
    float4 Color = 0;

    for (int i = 0; i < g_cKernelSize; i++)
    {    
        Color += tex2D( DownFilterRTSampler, In.TextureUV + PixelKernel[i].xy / g_ScreenDim * 4 ) * BlurWeights[i];
    }
    
    return Color;
}

float4 PS2( VS_OUTPUT In ) : COLOR0
{ 
    float4 Color = 0;

    for (int i = 0; i < g_cKernelSize; i++)
    {    
        Color += tex2D( DownFilterRTSampler, In.TextureUV + PixelKernel2[i].xy / g_ScreenDim * 4 ) * BlurWeights[i];
    }
    
    return Color;
}

float4 DofCombine( VS_OUTPUT In ) : COLOR0
{
    float3 ColorOrig = tex2D( OpaqueRTSampler, In.TextureUV );

    float3 ColorBlur = tex2D( DownFilterRTSampler, In.TextureUV );

    float Blur = dot( tex2D( PositionRTSampler, In.TextureUV ), FocalPlane );

    return float4( lerp( ColorOrig, ColorBlur, saturate(abs(Blur)) ), 1.0f );
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
    pass P1
    {          
        VertexShader = null;
        PixelShader  = compile ps_2_0 PS1(); 
    }
    pass P2
    {          
        VertexShader = null;
        PixelShader  = compile ps_2_0 PS2(); 
    }
    pass P3
    {          
        VertexShader = null;
        PixelShader  = compile ps_2_0 DofCombine(); 
	}
}