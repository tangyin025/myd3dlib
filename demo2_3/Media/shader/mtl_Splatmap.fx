#include "CommonVert.hlsl"

texture g_DiffuseTexture0:MaterialParameter<string path="texture/Checker.bmp";>;
texture g_NormalTexture0:MaterialParameter<string path="texture/Normal.dds";>;
texture g_SpecularTexture0:MaterialParameter<string path="texture/Gray.dds";>;
texture g_DiffuseTexture1:MaterialParameter<string path="texture/Red.dds";>;
texture g_NormalTexture1:MaterialParameter<string path="texture/Normal.dds";>;
texture g_SpecularTexture1:MaterialParameter<string path="texture/Gray.dds";>;
texture g_DiffuseTexture2:MaterialParameter<string path="texture/Green.dds";>;
texture g_NormalTexture2:MaterialParameter<string path="texture/Normal.dds";>;
texture g_SpecularTexture2:MaterialParameter<string path="texture/Gray.dds";>;
texture g_DiffuseTexture3:MaterialParameter<string path="texture/Blue.dds";>;
texture g_NormalTexture3:MaterialParameter<string path="texture/Normal.dds";>;
texture g_SpecularTexture3:MaterialParameter<string path="texture/Gray.dds";>;
float4 g_NormalStrength:MaterialParameter = float4(1.0, 1.0, 1.0, 1.0);
float2 g_TextureScale:MaterialParameter = float2(1.0, 1.0);

sampler DiffuseTextureSampler0 = sampler_state
{
    Texture = <g_DiffuseTexture0>;
    MipFilter = LINEAR;
    MinFilter = LINEAR;
    MagFilter = LINEAR;
    ADDRESSU = WRAP;
    ADDRESSV = WRAP;
};

sampler NormalTextureSampler0 = sampler_state
{
	Texture = <g_NormalTexture0>;
	MipFilter = LINEAR;
	MinFilter = LINEAR;
	MagFilter = LINEAR;
    ADDRESSU = WRAP;
    ADDRESSV = WRAP;
};

sampler SpecularTextureSampler0 = sampler_state
{
	Texture = <g_SpecularTexture0>;
	MipFilter = LINEAR;
	MinFilter = LINEAR;
	MagFilter = LINEAR;
    ADDRESSU = WRAP;
    ADDRESSV = WRAP;
};

sampler DiffuseTextureSampler1 = sampler_state
{
    Texture = <g_DiffuseTexture1>;
    MipFilter = LINEAR;
    MinFilter = LINEAR;
    MagFilter = LINEAR;
    ADDRESSU = WRAP;
    ADDRESSV = WRAP;
};

sampler NormalTextureSampler1 = sampler_state
{
	Texture = <g_NormalTexture1>;
	MipFilter = LINEAR;
	MinFilter = LINEAR;
	MagFilter = LINEAR;
    ADDRESSU = WRAP;
    ADDRESSV = WRAP;
};

sampler SpecularTextureSampler1 = sampler_state
{
	Texture = <g_SpecularTexture1>;
	MipFilter = LINEAR;
	MinFilter = LINEAR;
	MagFilter = LINEAR;
    ADDRESSU = WRAP;
    ADDRESSV = WRAP;
};

sampler DiffuseTextureSampler2 = sampler_state
{
    Texture = <g_DiffuseTexture2>;
    MipFilter = LINEAR;
    MinFilter = LINEAR;
    MagFilter = LINEAR;
    ADDRESSU = WRAP;
    ADDRESSV = WRAP;
};

sampler NormalTextureSampler2 = sampler_state
{
	Texture = <g_NormalTexture2>;
	MipFilter = LINEAR;
	MinFilter = LINEAR;
	MagFilter = LINEAR;
    ADDRESSU = WRAP;
    ADDRESSV = WRAP;
};

sampler SpecularTextureSampler2 = sampler_state
{
	Texture = <g_SpecularTexture2>;
	MipFilter = LINEAR;
	MinFilter = LINEAR;
	MagFilter = LINEAR;
    ADDRESSU = WRAP;
    ADDRESSV = WRAP;
};

sampler DiffuseTextureSampler3 = sampler_state
{
    Texture = <g_DiffuseTexture3>;
    MipFilter = LINEAR;
    MinFilter = LINEAR;
    MagFilter = LINEAR;
    ADDRESSU = WRAP;
    ADDRESSV = WRAP;
};

sampler NormalTextureSampler3 = sampler_state
{
	Texture = <g_NormalTexture3>;
	MipFilter = LINEAR;
	MinFilter = LINEAR;
	MagFilter = LINEAR;
    ADDRESSU = WRAP;
    ADDRESSV = WRAP;
};

sampler SpecularTextureSampler3 = sampler_state
{
	Texture = <g_SpecularTexture3>;
	MipFilter = LINEAR;
	MinFilter = LINEAR;
	MagFilter = LINEAR;
    ADDRESSU = WRAP;
    ADDRESSV = WRAP;
};

struct SHADOW_VS_OUTPUT
{
	float4 Pos				: POSITION;
	float2 Tex0				: TEXCOORD0;
};

SHADOW_VS_OUTPUT ShadowVS( VS_INPUT In )
{
    SHADOW_VS_OUTPUT Output;
	Output.Pos = TransformPos(In);
	Output.Tex0 = Output.Pos.zw;
    return Output;    
}

float4 ShadowPS( SHADOW_VS_OUTPUT In ) : COLOR0
{ 
    return In.Tex0.x / In.Tex0.y;
}

struct NORMAL_VS_OUTPUT
{
	float4 Pos				: POSITION;
	float4 Color			: COLOR0;
	float2 Tex0				: TEXCOORD0;
	float3 Normal			: NORMAL;
	float3 Tangent			: TEXCOORD1;
	float3 Binormal			: TEXCOORD2;
	float4 PosVS			: TEXCOORD3;
};

NORMAL_VS_OUTPUT NormalVS( VS_INPUT In )
{
	NORMAL_VS_OUTPUT Output;
	float4 PosWS = TransformPosWS(In);
	Output.Pos = mul(PosWS, g_ViewProj);
	Output.Color = TransformColor(In);
	Output.Tex0 = TransformUV(In) * g_TextureScale;
	Output.Normal = mul(TransformNormal(In), (float3x3)g_View);
	Output.Tangent = mul(TransformTangent(In), (float3x3)g_View);
	Output.Binormal = cross(Output.Normal, Output.Tangent);
	Output.PosVS = mul(PosWS, g_View);
	return Output;
}

void NormalPS( 	NORMAL_VS_OUTPUT In,
				out float4 oNormal : COLOR0,
				out float4 oSpecular : COLOR1,
				out float4 oPos : COLOR2 )
{
	float3x3 m = float3x3(normalize(In.Tangent), normalize(In.Binormal), normalize(In.Normal));
    float3 NormalTS = float3(0, 0, 0);
    float3 Specular = float3(0, 0, 0);
    if (In.Color.a >= 0.004)
    {
        NormalTS += (tex2D(NormalTextureSampler0, In.Tex0).rgb * 2 - 1) * float3(g_NormalStrength.x, g_NormalStrength.x, 1) * In.Color.a;
        Specular += tex2D(SpecularTextureSampler0, In.Tex0).xyz * In.Color.a;
    }
    if (In.Color.r >= 0.004)
    {
        NormalTS += (tex2D(NormalTextureSampler1, In.Tex0).rgb * 2 - 1) * float3(g_NormalStrength.y, g_NormalStrength.y, 1) * In.Color.r;
        Specular += tex2D(SpecularTextureSampler1, In.Tex0).xyz * In.Color.r;
    }
    if (In.Color.g >= 0.004)
    {
        NormalTS += (tex2D(NormalTextureSampler2, In.Tex0).rgb * 2 - 1) * float3(g_NormalStrength.z, g_NormalStrength.z, 1) * In.Color.g;
        Specular += tex2D(SpecularTextureSampler2, In.Tex0).xyz * In.Color.g;
    }
    if (In.Color.b >= 0.004)
    {
        NormalTS += (tex2D(NormalTextureSampler3, In.Tex0).rgb * 2 - 1) * float3(g_NormalStrength.w, g_NormalStrength.w, 1) * In.Color.b;
        Specular += tex2D(SpecularTextureSampler3, In.Tex0).xyz * In.Color.b;
    }
    oNormal = float4(mul(normalize(NormalTS), m), 1);
	oSpecular = float4(Specular, 1);
	oPos = In.PosVS;
}

struct OPAQUE_VS_OUTPUT
{
	float4 Pos				: SV_Position;
	float2 Tex0				: TEXCOORD0;
	float4 Color			: COLOR0;
	float3 ViewVS			: TEXCOORD4;
	float4 PosWS			: TEXCOORD2;
	float InvScreenDepth	: TEXCOORD3;
	float fogFactor			: TEXCOORD1;
};

OPAQUE_VS_OUTPUT OpaqueVS( VS_INPUT In )
{
    OPAQUE_VS_OUTPUT Output;
	Output.PosWS = TransformPosWS(In);
	Output.Pos = mul(Output.PosWS, g_ViewProj);
	Output.InvScreenDepth = Output.Pos.w / Output.Pos.z;
	Output.Tex0 = TransformUV(In) * g_TextureScale;
    Output.Color = TransformColor(In);
    Output.ViewVS = mul(Output.PosWS.xyz - g_Eye, (float3x3)g_View); // ! dont normalize here
	Output.fogFactor = GetFogFactor(length(Output.ViewVS));
    return Output;    
}

float4 OpaquePS( OPAQUE_VS_OUTPUT In ) : COLOR0
{
    float3 SkyLightDirVS = mul(g_SkyLightDir, (float3x3)g_View);
    float3 NormalVS = tex2D(NormalRTSampler, (In.Pos.xy + 0.5f) / g_ScreenDim).xyz;
    float SkyLightAmount = saturate(GetLigthAmount(In.PosWS, In.InvScreenDepth) * dot(NormalVS, SkyLightDirVS));
    float3 SkyDiffuse = g_SkyLightColor.xyz * SkyLightAmount;
    float3 Ref = reflect(normalize(In.ViewVS), NormalVS);
    float3 SpecularVS = tex2D(SpecularRTSampler, (In.Pos.xy + 0.5f) / g_ScreenDim).xyz;
    float SkySpecular = DistributionGGX(SkyLightDirVS, Ref, SpecularVS.r) * g_SkyLightColor.w * SpecularVS.g * SkyLightAmount;
    float4 Diffuse = float4(0,0,0,0);
	if (In.Color.a >= 0.004)
        Diffuse += tex2D(DiffuseTextureSampler0, In.Tex0) * In.Color.a;
	if (In.Color.r >= 0.004)
		Diffuse += tex2D(DiffuseTextureSampler1, In.Tex0) * In.Color.r;
	if (In.Color.g >= 0.004)
		Diffuse += tex2D(DiffuseTextureSampler2, In.Tex0) * In.Color.g;
	if (In.Color.b >= 0.004)
		Diffuse += tex2D(DiffuseTextureSampler3, In.Tex0) * In.Color.b;
	float4 ScreenLight = tex2D(LightRTSampler, (In.Pos.xy + 0.5f) / g_ScreenDim);
    float3 fres = g_AmbientColor.xyz * Fresnel(NormalVS, normalize(In.ViewVS), 5, SpecularVS.g);
	float3 Final = Diffuse.xyz * (ScreenLight.xyz + SkyDiffuse) + ScreenLight.w + SkySpecular + fres;
    return float4(lerp(Final, g_FogColor.xyz, In.fogFactor), 1);
}

technique RenderScene
{
    pass PassTypeShadow
    {          
        VertexShader = compile vs_3_0 ShadowVS();
        PixelShader  = compile ps_3_0 ShadowPS(); 
    }
    pass PassTypeNormal
    {          
        VertexShader = compile vs_3_0 NormalVS();
        PixelShader  = compile ps_3_0 NormalPS(); 
    }
    pass PassTypeLight
    {          
    }
	pass PassTypeBackground
	{
	}
    pass PassTypeOpaque
    {          
        VertexShader = compile vs_3_0 OpaqueVS();
        PixelShader  = compile ps_3_0 OpaquePS(); 
    }
    pass PassTypeTransparent
    {          
    }
}
