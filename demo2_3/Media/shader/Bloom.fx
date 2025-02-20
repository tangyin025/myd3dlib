
#include "CommonHeader.hlsl"

static const int g_cKernelSize = 13;

float _LuminanceThreshold = 0.5;

float3 _BloomColor = float3( 1.0, 1.0, 1.0 );

float _BloomFactor = 1.0;

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

float4 ExtractBright(VS_OUTPUT In) : COLOR0
{
    float4 Color = tex2D( OpaqueRTSampler, In.TextureUV );
	float v = clamp(0.2125 * Color.r + 0.7154 * Color.g + 0.0721 * Color.b - _LuminanceThreshold, 0.0, 1.0);
	return float4(Color.rgb * v, 1.0);
}

float4 ColorBGlurH( VS_OUTPUT In ) : COLOR0
{ 
    float4 Color = 0;

    for (int i = 0; i < g_cKernelSize; i++)
    {    
        Color += tex2D( DownFilterRTSampler, In.TextureUV + PixelKernelH[i].xy / g_ScreenDim ) * BlurWeights[i];
    }
    
    return Color;
}

float4 ColorBGlurV( VS_OUTPUT In ) : COLOR0
{ 
    float4 Color = 0;

    for (int i = 0; i < g_cKernelSize; i++)
    {    
        Color += tex2D( DownFilterRTSampler, In.TextureUV + PixelKernelV[i].xy / g_ScreenDim ) * BlurWeights[i];
    }
    
    return Color;
}

float4 BloomPS( VS_OUTPUT In ) : COLOR0
{
    float3 ColorOrig = tex2D( OpaqueRTSampler, In.TextureUV );

    float3 ColorBlur = tex2D( DownFilterRTSampler, In.TextureUV );
	
	float3 Color = ColorOrig + ColorBlur * _BloomColor * _BloomFactor;

    return float4(Color, 1.0);
}

//--------------------------------------------------------------------------------------
// Renders scene 
//--------------------------------------------------------------------------------------
technique RenderScene
{
    pass P0
    {
		VertexShader = null;
		PixelShader = compile ps_3_0 ExtractBright();
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
        PixelShader  = compile ps_3_0 BloomPS(); 
	}
}
