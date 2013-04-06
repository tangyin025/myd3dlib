
#include "CommonHeader.fx"

struct VS_OUTPUT
{
    float4 Position   : POSITION;   // vertex position 
    float4 Diffuse    : COLOR0;     // vertex diffuse color (note that COLOR0 is clamped from 0..1)
    float2 TextureUV  : TEXCOORD0;  // vertex texture coords 
};

VS_OUTPUT RenderSceneVS( float2 vTexCoord0 : TEXCOORD0, 
						 float4 vPos : POSITION0,
                         float4 vDiffuse : COLOR0 )
{
    VS_OUTPUT Output;
	float4 vParticlePos = float4(lerp(-.5,.5,vTexCoord0.x), lerp(.5,-.5,vTexCoord0.y), 0, 1);
	Output.Position = mul(vParticlePos + vPos, g_mWorldViewProjection);
	
	Output.Diffuse = vDiffuse;
	Output.TextureUV = vTexCoord0;
    
    return Output;    
}

float4 RenderScenePS( VS_OUTPUT In ) : COLOR0
{ 
    return In.Diffuse;
}

technique RenderScene
{
    pass P0
    {          
		CullMode = NONE;
        VertexShader = compile vs_2_0 RenderSceneVS();
        PixelShader  = compile ps_2_0 RenderScenePS(); 
    }
}
