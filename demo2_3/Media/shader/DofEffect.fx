
#include "CommonHeader.hlsl"

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
	return f;
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
        VertexShader = null;
        PixelShader  = compile ps_3_0 ColorBGlurH(); 
    }
    pass P1
    {          
        VertexShader = null;
        PixelShader  = compile ps_3_0 ColorBGlurV(); 
    }
    pass P2
    {          
        VertexShader = null;
        PixelShader  = compile ps_3_0 DofCombine(); 
	}
}
