#include "CommonVert.hlsl"

texture g_NormalTexture : MaterialParameter < string path = "texture/WaterNormal2.png";
> ;
texture g_ReflectTexture : MaterialParameter < string path = "texture/galileo_cross.dds";
> ;
float fresnelBias : MaterialParameter = -0.1;
float fresnelScale : MaterialParameter = 1.8;
float fresnelPower : MaterialParameter = 8;

sampler NormalTextureSampler = sampler_state
{
	Texture = <g_NormalTexture>;
	MipFilter = LINEAR;
	MinFilter = LINEAR;
	MagFilter = LINEAR;
    ADDRESSU = WRAP;
    ADDRESSV = WRAP;
};

sampler ReflectTextureSampler = sampler_state
{
    Texture = <g_ReflectTexture>;
    MipFilter = LINEAR;
    MinFilter = LINEAR;
    MagFilter = LINEAR;
    ADDRESSU = CLAMP;
    ADDRESSV = CLAMP;
};

struct TRANSPARENT_VS_OUTPUT
{
    float4 Pos : SV_POSITION;
    float2 texCoord0 : TEXCOORD0;
    float2 texCoord1 : TEXCOORD1;
    float2 texCoord2 : TEXCOORD2;
    float3 ViewWS : TEXCOORD3;
	float fogFactor : TEXCOORD4;
};

TRANSPARENT_VS_OUTPUT TransparentVS(VS_INPUT In)
{
    TRANSPARENT_VS_OUTPUT Output;
    float4 PosWS = TransformPosWS(In);
    Output.Pos = mul(PosWS, g_ViewProj);
    float2 Tex0 = TransformUV(In);
    Output.texCoord0 = Tex0 + g_Time * 0.05;
    Output.texCoord1 = Tex0 * 2.0 + g_Time * -0.05;
    Output.texCoord2 = Tex0 / 2.0 + g_Time * 0.05;
    Output.ViewWS = PosWS.xyz - g_Eye;
	float3 ViewVS = mul(Output.ViewWS, (float3x3)g_View); // ! dont normalize here
	Output.fogFactor = GetFogFactor(length(ViewVS));
    return Output;
}

float4 TransparentPS(TRANSPARENT_VS_OUTPUT In)
    : COLOR
{
    // 波纹扰动
    float3x3 m = {
		1, 0, 0,
		0, 0, 1,
		0, 1, 0
	};
    float3 nt0 = tex2D(NormalTextureSampler, In.texCoord0).xyz;
    float3 nt1 = tex2D(NormalTextureSampler, In.texCoord1).xyz;
    float3 nt2 = tex2D(NormalTextureSampler, In.texCoord2).xyz;
    float3 nt = normalize(2.0 * (nt0 + nt1 + nt2) - 3.0);
    float3 Normal = mul(nt, m);

    // 反射
    float3 i = normalize(In.ViewWS);
    float3 refl = reflect(i, Normal);
    float3 reflcolor = texCUBE(ReflectTextureSampler, float3(refl.x, refl.y, -refl.z)).xyz; // 右手系修正

    // 折射
	float2 screen = (In.Pos.xy + 0.5f) / g_ScreenDim + Normal.xz * 0.1;
    float3 refrcolor = tex2D(OpaqueRTSampler, screen).xyz;

    // 菲涅尔
    float fresnel = saturate(fresnelBias + fresnelScale * pow(1 + dot(i, Normal), fresnelPower));
    return float4(lerp(lerp(refrcolor, reflcolor, fresnel), g_FogColor.xyz, In.fogFactor), 1);
}

technique RenderScene
{
    pass PassTypeShadow
    {
    }
    pass PassTypeNormal
    {
    }
    pass PassTypeLight
    {
    }
    pass PassTypeBackground
    {
    }
    pass PassTypeOpaque
    {
    }
    pass PassTypeTransparent
    {
        VertexShader = compile vs_3_0 TransparentVS();
        PixelShader = compile ps_3_0 TransparentPS();
    }
}
