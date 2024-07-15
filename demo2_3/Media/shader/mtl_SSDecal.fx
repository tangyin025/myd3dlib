#if defined(INSTANCE) || defined(EMITTER_FACE_TYPE)
#error Cannot use Decal with instance or particle system, because of invalid g_InvWorldView
#endif
#include "CommonVert.hlsl"

texture g_DiffuseTexture:MaterialParameter<string path="texture/Checker.bmp";>;
texture g_NormalTexture:MaterialParameter<string path="texture/Normal.dds";>;
texture g_SpecularTexture:MaterialParameter<string path="texture/Gray.dds";>;
float g_NormalStrength:MaterialParameter=1.0;
float4x4 g_InvWorldView:MaterialParameter<bool UseInvWorldView=true;>;

sampler DiffuseTextureSampler = sampler_state
{
    Texture = <g_DiffuseTexture>;
    MipFilter = LINEAR;
    MinFilter = LINEAR;
    MagFilter = LINEAR;
    ADDRESSU = CLAMP;
    ADDRESSV = CLAMP;
};

sampler NormalTextureSampler = sampler_state
{
	Texture = <g_NormalTexture>;
	MipFilter = LINEAR;
	MinFilter = LINEAR;
	MagFilter = LINEAR;
    ADDRESSU = CLAMP;
    ADDRESSV = CLAMP;
};

sampler SpecularTextureSampler = sampler_state
{
	Texture = <g_SpecularTexture>;
	MipFilter = LINEAR;
	MinFilter = LINEAR;
	MagFilter = LINEAR;
    ADDRESSU = CLAMP;
    ADDRESSV = CLAMP;
};

struct NORMAL_VS_OUTPUT
{
	float4 Pos				: SV_Position;
	float4 Color			: COLOR0;
	float2 Tex0				: TEXCOORD0;
	float3 Normal			: NORMAL;
	float3 Tangent			: TEXCOORD1;
	float3 Binormal			: TEXCOORD2;
	float3 PosVS			: TEXCOORD3;
};

NORMAL_VS_OUTPUT NormalVS( VS_INPUT In )
{
	NORMAL_VS_OUTPUT Output;
	float4 PosWS = TransformPosWS(In);
	Output.Pos = mul(PosWS, g_ViewProj);
	Output.Color = TransformColor(In);
	Output.Tex0 = TransformUV(In);
	Output.Normal = mul(normalize(mul(float3(0, 1, 0), (float3x3)g_World)), (float3x3)g_View);
	Output.Tangent = mul(normalize(mul(float3(1, 0, 0), (float3x3)g_World)), (float3x3)g_View);
	Output.Binormal = cross(Output.Normal, Output.Tangent);
	Output.PosVS = mul(PosWS, g_View).xyz;
	return Output;
}

void NormalPS( 	NORMAL_VS_OUTPUT In,
				out float4 oNormal : COLOR0,
				out float4 oSpecular : COLOR1 )
{
	float4 PosVS = tex2D(PositionRTSampler, (In.Pos.xy + 0.5f) / g_ScreenDim);
	float4 Pos = mul(PosVS, g_InvWorldView);
	// ! clip sky box with invalid w
	Pos.xyz /= Pos.w;
	clip(float3(0.5, 0.5, 0.5) - abs(Pos));

    float3x3 m = float3x3(normalize(In.Tangent), normalize(In.Binormal), normalize(In.Normal));
    float3 NormalTS = normalize((tex2D(NormalTextureSampler, Pos.xz + 0.5).xyz * 2 - 1) * float3(g_NormalStrength, g_NormalStrength, 1));
	float4 Diffuse = tex2D(DiffuseTextureSampler, Pos.xz + 0.5);
    oNormal = float4(mul(NormalTS, m), In.Color.w * Diffuse.a);
    float3 Specular = tex2D(SpecularTextureSampler, Pos.xz + 0.5).xyz;
    oSpecular = float4(Specular, In.Color.w * Diffuse.a);
}

struct OPAQUE_VS_OUTPUT
{
	float4 Pos				: SV_Position;
	float4 Color			: COLOR0;
	float2 Tex0				: TEXCOORD0;
	float3 ViewVS			: TEXCOORD1;
	float4 PosWS			: TEXCOORD2;
	float InvScreenDepth	: TEXCOORD3;
};

OPAQUE_VS_OUTPUT OpaqueVS( VS_INPUT In )
{
    OPAQUE_VS_OUTPUT Output;
	Output.PosWS = TransformPosWS(In);
	Output.Pos = mul(Output.PosWS, g_ViewProj);
	Output.InvScreenDepth = Output.Pos.w / Output.Pos.z;
	Output.Color = TransformColor(In);
    Output.Tex0 = TransformUV(In);
    Output.ViewVS = mul(Output.PosWS.xyz - g_Eye, (float3x3)g_View); // ! dont normalize here
    return Output;    
}

float4 OpaquePS( OPAQUE_VS_OUTPUT In ) : COLOR0
{ 
	float4 PosVS = tex2D(PositionRTSampler, (In.Pos.xy + 0.5f) / g_ScreenDim);
	float4 Pos = mul(PosVS, g_InvWorldView);
	// ! clip sky box with invalid w
	Pos.xyz /= Pos.w;
	clip(float3(0.5, 0.5, 0.5) - abs(Pos));

	float3 SkyLightDirVS = mul(g_SkyLightDir, (float3x3)g_View);
    float3 NormalVS = normalize(tex2D(NormalRTSampler, (In.Pos.xy + 0.5f) / g_ScreenDim).xyz);
    float SkyLightAmount = saturate(GetLigthAmount(In.PosWS, In.InvScreenDepth) * dot(NormalVS, SkyLightDirVS));
    float3 SkyDiffuse = g_SkyLightColor.xyz * SkyLightAmount;
    float3 Ref = reflect(normalize(In.ViewVS), NormalVS);
    float3 Specular = tex2D(SpecularTextureSampler, In.Tex0).xyz;
    float SkySpecular = DistributionGGX(SkyLightDirVS, Ref, Specular.r) * g_SkyLightColor.w * Specular.g * SkyLightAmount;
    float4 Diffuse = tex2D(DiffuseTextureSampler, Pos.xz + 0.5);
	float4 ScreenLight = tex2D(LightRTSampler, (In.Pos.xy + 0.5f) / g_ScreenDim);
    float3 fres = g_AmbientColor.xyz * Fresnel(NormalVS, normalize(In.ViewVS), 5, Specular.g);
	float3 Final = Diffuse.xyz * In.Color.xyz * (ScreenLight.xyz + SkyDiffuse) + ScreenLight.w + SkySpecular + fres;
    return float4(Final, In.Color.w * Diffuse.a);
}

technique RenderScene
{
    pass PassTypeShadow
    {          
    }
    pass PassTypeNormal
    {          
    }
    pass PassTypeNormalTrans
    {          
		BlendOp = ADD;
		SrcBlend = SRCALPHA;
		DestBlend = ONE;
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
    }
    pass PassTypeTransparent
    {          
        VertexShader = compile vs_3_0 OpaqueVS();
        PixelShader  = compile ps_3_0 OpaquePS(); 
    }
}
