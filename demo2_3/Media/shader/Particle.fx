
#include "CommonHeader.fx"

//--------------------------------------------------------------------------------------
// Global variables
//--------------------------------------------------------------------------------------

float3 g_ParticleDir;
float3 g_ParticleUp;
float3 g_ParticleRight;
float2 g_AnimationColumnRow;

//--------------------------------------------------------------------------------------
// Vertex shader output structure
//--------------------------------------------------------------------------------------

struct VS_OUTPUT_PARTICLE
{
    float4 Position   : POSITION;   // vertex position 
    float4 Diffuse    : COLOR0;     // vertex diffuse color (note that COLOR0 is clamped from 0..1)
    float2 TextureUV  : TEXCOORD0;  // vertex texture coords 
};

//--------------------------------------------------------------------------------------
// rotate_angle_axis
//--------------------------------------------------------------------------------------

float3 rotate_angle_axis(float3 v, float a, float3 N)
{
	float sin_a, cos_a;
	sincos(a, sin_a, cos_a);
	float Nxx = N.x * N.x;
	float Nyy = N.y * N.y;
	float Nzz = N.z * N.z;
	float Nxy = N.x * N.y;
	float Nyz = N.y * N.z;
	float Nzx = N.z * N.x;
	
	float3x3 mRotation = {
		Nxx * (1 - cos_a) + cos_a,			Nxy * (1 - cos_a) + N.z * sin_a,	Nzx * (1 - cos_a) - N.y * sin_a,
		Nxy * (1 - cos_a) - N.z * sin_a,	Nyy * (1 - cos_a) + cos_a,			Nyz * (1 - cos_a) + N.x * sin_a,
		Nzx * (1 - cos_a) + N.y * sin_a,	Nyz * (1 - cos_a) - N.x * sin_a,	Nzz * (1 - cos_a) + cos_a};
		
	return mul(v, mRotation);
}

//--------------------------------------------------------------------------------------
// This shader computes standard transform and lighting
//--------------------------------------------------------------------------------------

VS_OUTPUT_PARTICLE RenderSceneVS( float2 vTexCoord0 : TEXCOORD0, 
						 float4 vPos : POSITION0,
                         float4 vDiffuse : COLOR0,
						 float4 vTexCoord1 : TEXCOORD1,
						 float4 vTexCoord2 : TEXCOORD2 )
{
    VS_OUTPUT_PARTICLE Output;
	float4 LocalPos = float4(rotate_angle_axis(
		g_ParticleUp * lerp(vTexCoord1.y * 0.5, -vTexCoord1.y * 0.5, vTexCoord0.y) +
		g_ParticleRight * lerp(-vTexCoord1.x * 0.5, vTexCoord1.x * 0.5, vTexCoord0.x), vTexCoord1.z, g_ParticleDir), 0);
		
	Output.Position = mul(LocalPos + vPos, mul(g_World, g_ViewProj));
	
	Output.Diffuse = vDiffuse;
	Output.TextureUV = (vTexCoord2.xy + vTexCoord0.xy) / g_AnimationColumnRow;
    
    return Output;    
}

//--------------------------------------------------------------------------------------
// This shader outputs the pixel's color by modulating the texture's
// color with diffuse material color
//--------------------------------------------------------------------------------------

float4 RenderScenePS( VS_OUTPUT_PARTICLE In ) : COLOR0
{ 
    return tex2D(MeshTextureSampler, In.TextureUV) * In.Diffuse;
}

//--------------------------------------------------------------------------------------
// Renders scene 
//--------------------------------------------------------------------------------------

technique RenderScene
{
    pass P0
    {          
		// FILLMODE = WIREFRAME;
		CullMode = NONE;
		Lighting = FALSE;
		AlphaBlendEnable = TRUE;
		BlendOp = ADD;
		SrcBlend = SRCCOLOR;
		DestBlend = ONE;
		ZEnable = TRUE;
		ZWriteEnable = FALSE;
        VertexShader = compile vs_2_0 RenderSceneVS();
        PixelShader  = compile ps_2_0 RenderScenePS(); 
    }
}
