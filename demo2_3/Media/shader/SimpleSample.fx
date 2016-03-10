//--------------------------------------------------------------------------------------
// File: SimpleSample.fx
//
// The effect file for the SimpleSample sample.  
// 
// Copyright (c) Microsoft Corporation. All rights reserved.
//--------------------------------------------------------------------------------------

#include "CommonHeader.fx"

float4 g_WireColor = float4(0,1,0,1);

static const int g_cKernelSize = 13;

float4 g_DofParams = float4( 5.0f, 15.0f, 25.0f, 1.0f );

float2 PixelKernelH[g_cKernelSize] =
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

float2 PixelKernelV[g_cKernelSize] =
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
    Output.Position = mul(mul(vPos, g_World), g_ViewProj);
    
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
    // return tex2D(DownFilterRTSampler, In.TextureUV);
	return g_WireColor;
}

float4 ColorBGlurH( VS_OUTPUT In ) : COLOR0
{ 
    float4 Color = 0;

    for (int i = 0; i < g_cKernelSize; i++)
    {    
        Color += tex2D( DownFilterRTSampler, In.TextureUV + PixelKernelH[i].xy / g_ScreenDim * 4 ) * BlurWeights[i];
    }
    
    return Color;
}

float4 ColorBGlurV( VS_OUTPUT In ) : COLOR0
{ 
    float4 Color = 0;

    for (int i = 0; i < g_cKernelSize; i++)
    {    
        Color += tex2D( DownFilterRTSampler, In.TextureUV + PixelKernelV[i].xy / g_ScreenDim * 4 ) * BlurWeights[i];
    }
    
    return Color;
}

float ComputeDepthBlur(float depth)
{
	float f;
	if (depth < g_DofParams.y)
	{
		f = (g_DofParams.y - depth) / (g_DofParams.y - g_DofParams.x);
	}
	else
	{
		f = (depth - g_DofParams.y) / (g_DofParams.z - g_DofParams.y);
		f = clamp(f, 0, g_DofParams.w);
	}
	return f * 0.5 + 0.5;
}

float4 DofCombine( VS_OUTPUT In ) : COLOR0
{
    float3 ColorOrig = tex2D( OpaqueRTSampler, In.TextureUV );

    float3 ColorBlur = tex2D( DownFilterRTSampler, In.TextureUV );

    float Blur = ComputeDepthBlur(-tex2D( PositionRTSampler, In.TextureUV ).z);

    return float4( lerp( ColorOrig, ColorBlur, saturate(Blur) ), 1.0f );
}

//--------------------------------------------------------------------------------------
// Renders scene 
//--------------------------------------------------------------------------------------
technique RenderScene
{
    pass P0
    {          
		FillMode = WIREFRAME;
        VertexShader = compile vs_3_0 RenderSceneVS();
        PixelShader  = compile ps_3_0 RenderScenePS(); 
    }
    pass P1
    {          
        VertexShader = null;
        PixelShader  = compile ps_3_0 ColorBGlurH(); 
    }
    pass P2
    {          
        VertexShader = null;
        PixelShader  = compile ps_3_0 ColorBGlurV(); 
    }
    pass P3
    {          
        VertexShader = null;
        PixelShader  = compile ps_3_0 DofCombine(); 
	}
}