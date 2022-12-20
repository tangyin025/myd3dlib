
texture g_DiffuseTexture:MaterialParameter<string path="texture/Checker.bmp";>;
texture g_NormalTexture:MaterialParameter<string path="texture/Normal.dds";>;
texture g_SpecularTexture:MaterialParameter<string path="texture/White.dds";>;
float g_Shininess:MaterialParameter = 25;

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
	clip(ScreenDoorTransparency(In.Color.w, In.Pos.xy));
	float3x3 m = float3x3(In.Tangent, In.Binormal, In.Normal);
	float3 NormalTS = tex2D(NormalTextureSampler, In.Tex0).xyz * 2 - 1;
	oNormal = float4(mul(NormalTS, m), 1);
	oSpecular = float4(g_Shininess, 0, 0, 1);
	oPos = float4(In.PosVS, 1.0);
}

struct OPAQUE_VS_OUTPUT
{
	float4 Pos				: SV_Position;
	float4 Color			: COLOR0;
	float2 Tex0				: TEXCOORD0;
	float4 ShadowCoord		: TEXCOORD1;
	float3 ViewVS			: TEXCOORD2;
};

OPAQUE_VS_OUTPUT OpaqueVS( VS_INPUT In )
{
    OPAQUE_VS_OUTPUT Output;
	float4 PosWS = TransformPosWS(In);
	Output.Pos = mul(PosWS, g_ViewProj);
	Output.Color = TransformColor(In);
	Output.Tex0 = TransformUV(In);
	Output.ShadowCoord = mul(PosWS, g_SkyLightViewProj);
	Output.ViewVS = mul(g_Eye - PosWS.xyz, (float3x3)g_View); // ! dont normalize here
    return Output;    
}

float4 OpaquePS( OPAQUE_VS_OUTPUT In ) : COLOR0
{ 
	clip(ScreenDoorTransparency(In.Color.w, In.Pos.xy));
	float3 SkyLightDir = normalize(float3(g_SkyLightView[0][2], g_SkyLightView[1][2], g_SkyLightView[2][2]));
	float3 SkyLightDirVS = mul(SkyLightDir, (float3x3)g_View);
	float LightAmount = GetLigthAmount(In.ShadowCoord);
	float3 NormalVS = tex2D(NormalRTSampler, (In.Pos.xy + 0.5f) / g_ScreenDim).xyz;
	float3 SkyDiffuse = saturate(dot(NormalVS, SkyLightDirVS) * LightAmount) * g_SkyLightColor.xyz;
	float3 Ref = Reflection(NormalVS, In.ViewVS);
	float SkySpecular = pow(saturate(dot(Ref, SkyLightDirVS) * LightAmount), g_Shininess) * g_SkyLightColor.w;
	float4 Diffuse = tex2D(DiffuseTextureSampler, In.Tex0);
	float3 Specular = tex2D(SpecularTextureSampler, In.Tex0).xyz;
	float4 ScreenLight = tex2D(LightRTSampler, (In.Pos.xy + 0.5f) / g_ScreenDim);
	float3 Final = Diffuse.xyz * In.Color.xyz * (ScreenLight.xyz + SkyDiffuse) + Specular * (ScreenLight.w + SkySpecular);
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
