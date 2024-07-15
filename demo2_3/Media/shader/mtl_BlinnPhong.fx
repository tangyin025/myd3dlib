#include "CommonVert.hlsl"

texture g_DiffuseTexture:MaterialParameter<string path="texture/Checker.bmp";>;
texture g_NormalTexture:MaterialParameter<string path="texture/Normal.dds";>;
texture g_SpecularTexture:MaterialParameter<string path="texture/Gray.dds";>;
float g_NormalStrength:MaterialParameter=1.0;

sampler DiffuseTextureSampler = sampler_state
{
    Texture = <g_DiffuseTexture>;
    MipFilter = LINEAR;
    MinFilter = LINEAR;
    MagFilter = LINEAR;
};

sampler NormalTextureSampler = sampler_state
{
	Texture = <g_NormalTexture>;
	MipFilter = LINEAR;
	MinFilter = LINEAR;
	MagFilter = LINEAR;
};

sampler SpecularTextureSampler = sampler_state
{
	Texture = <g_SpecularTexture>;
	MipFilter = LINEAR;
	MinFilter = LINEAR;
	MagFilter = LINEAR;
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
	Output.Normal = mul(TransformNormal(In), (float3x3)g_View);
	Output.Tangent = mul(TransformTangent(In), (float3x3)g_View);
	Output.Binormal = cross(Output.Normal, Output.Tangent);
	Output.PosVS = mul(PosWS, g_View).xyz;
	return Output;
}

void NormalPS( 	NORMAL_VS_OUTPUT In,
				out float4 oNormal : COLOR0,
				out float4 oSpecular : COLOR1,
				out float4 oPos : COLOR2 )
{
	// clip(ScreenDoorTransparency(In.Color.w, In.Pos.xy));
	float3x3 m = float3x3(normalize(In.Tangent), normalize(In.Binormal), normalize(In.Normal));
	float3 NormalTS = normalize((tex2D(NormalTextureSampler, In.Tex0).xyz * 2 - 1) * float3(g_NormalStrength, g_NormalStrength, 1));
	oNormal = float4(mul(NormalTS, m), 1);
	float3 Specular = tex2D(SpecularTextureSampler, In.Tex0).xyz;
	oSpecular = float4(Specular, 1);
	oPos = float4(In.PosVS, 1.0);
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
	// clip(ScreenDoorTransparency(In.Color.w, In.Pos.xy));
    float3 SkyLightDirVS = mul(g_SkyLightDir, (float3x3)g_View);
    float3 NormalVS = tex2D(NormalRTSampler, (In.Pos.xy + 0.5f) / g_ScreenDim).xyz;
    float SkyLightAmount = saturate(GetLigthAmount(In.PosWS, In.InvScreenDepth) * dot(NormalVS, SkyLightDirVS));
    float3 SkyDiffuse = g_SkyLightColor.xyz * SkyLightAmount;
    float3 Ref = reflect(normalize(In.ViewVS), NormalVS);
    float3 Specular = tex2D(SpecularTextureSampler, In.Tex0).xyz;
    float SkySpecular = DistributionGGX(SkyLightDirVS, Ref, Specular.r) * g_SkyLightColor.w * Specular.g * SkyLightAmount;
	float4 Diffuse = tex2D(DiffuseTextureSampler, In.Tex0);
	float4 ScreenLight = tex2D(LightRTSampler, (In.Pos.xy + 0.5f) / g_ScreenDim);
    float3 fres = g_AmbientColor.xyz * Fresnel(NormalVS, normalize(In.ViewVS), 5, Specular.g);
	float3 Final = Diffuse.xyz * In.Color.xyz * (ScreenLight.xyz + SkyDiffuse) + ScreenLight.w + SkySpecular + fres;
    return float4(Final, 1);
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
    pass PassTypeNormalTrans
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
        VertexShader = compile vs_3_0 OpaqueVS();
        PixelShader  = compile ps_3_0 OpaquePS(); 
    }
    pass PassTypeTransparent
    {          
    }
}
