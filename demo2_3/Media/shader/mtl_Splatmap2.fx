
texture g_NormalTex:MaterialParameter<string path="texture/Normal.dds";>;
texture g_WeightTex:MaterialParameter<string path="texture/Black.dds";>;
texture g_DiffuseTexture0:MaterialParameter<string path="texture/Red.dds";>;
texture g_NormalTexture0:MaterialParameter<string path="texture/Normal.dds";>;
texture g_SpecularTexture0:MaterialParameter<string path="texture/White.dds";>;
texture g_DiffuseTexture1:MaterialParameter<string path="texture/Green.dds";>;
texture g_NormalTexture1:MaterialParameter<string path="texture/Normal.dds";>;
texture g_SpecularTexture1:MaterialParameter<string path="texture/White.dds";>;
texture g_DiffuseTexture2:MaterialParameter<string path="texture/Blue.dds";>;
texture g_NormalTexture2:MaterialParameter<string path="texture/Normal.dds";>;
texture g_SpecularTexture2:MaterialParameter<string path="texture/White.dds";>;
texture g_DiffuseTexture3:MaterialParameter<string path="texture/Checker.bmp";>;
texture g_NormalTexture3:MaterialParameter<string path="texture/Normal.dds";>;
texture g_SpecularTexture3:MaterialParameter<string path="texture/White.dds";>;
float g_Shininess:MaterialParameter = 25;
float2 g_TextureScale:MaterialParameter = float2(1.0, 1.0);

sampler NormalTexSampler = sampler_state
{
    Texture = <g_NormalTex>;
    MipFilter = LINEAR;
    MinFilter = LINEAR;
    MagFilter = LINEAR;
};

sampler WeightTexSampler = sampler_state
{
	Texture = <g_WeightTex>;
	MipFilter = LINEAR;
	MinFilter = LINEAR;
	MagFilter = LINEAR;
};

sampler DiffuseTextureSampler0 = sampler_state
{
    Texture = <g_DiffuseTexture0>;
    MipFilter = LINEAR;
    MinFilter = LINEAR;
    MagFilter = LINEAR;
};

sampler NormalTextureSampler0 = sampler_state
{
	Texture = <g_NormalTexture0>;
	MipFilter = LINEAR;
	MinFilter = LINEAR;
	MagFilter = LINEAR;
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
};

sampler NormalTextureSampler1 = sampler_state
{
	Texture = <g_NormalTexture1>;
	MipFilter = LINEAR;
	MinFilter = LINEAR;
	MagFilter = LINEAR;
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
};

sampler NormalTextureSampler2 = sampler_state
{
	Texture = <g_NormalTexture2>;
	MipFilter = LINEAR;
	MinFilter = LINEAR;
	MagFilter = LINEAR;
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
};

sampler NormalTextureSampler3 = sampler_state
{
	Texture = <g_NormalTexture3>;
	MipFilter = LINEAR;
	MinFilter = LINEAR;
	MagFilter = LINEAR;
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
	Output.Pos = TransformPosShadow(In);
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
	float3 PosVS			: TEXCOORD3;
    float2 Tex1             : TEXCOORD4;
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
    tex2D(NormalRTSampler, (In.Pos.xy + 0.5f) / g_ScreenDim).xyz;
	Output.PosVS = mul(PosWS, g_View).xyz;
    Output.Tex1 = TransformUV(In);
	return Output;
}

void NormalPS( 	NORMAL_VS_OUTPUT In,
				out float4 oNormal : COLOR0,
				out float4 oSpecular : COLOR1,
				out float4 oPos : COLOR2 )
{
	float3x3 m = float3x3(In.Tangent, In.Binormal, In.Normal);
	float3 NormalTS = tex2D(NormalTexSampler, In.Tex1).rgb * 2 - 1;
    float4 Color = tex2D(WeightTexSampler, In.Tex1);
	if (Color.r >= 0.004)
		NormalTS += (tex2D(NormalTextureSampler0, In.Tex0).rgb * 2 - 1) * Color.r;
	if (Color.g >= 0.004)
		NormalTS += (tex2D(NormalTextureSampler1, In.Tex0).rgb * 2 - 1) * Color.g;
	if (Color.b >= 0.004)
		NormalTS += (tex2D(NormalTextureSampler2, In.Tex0).rgb * 2 - 1) * Color.b;
	if (Color.a >= 0.004)
		NormalTS += (tex2D(NormalTextureSampler3, In.Tex0).rgb * 2 - 1) * Color.a;
	oNormal = float4(mul(normalize(NormalTS), m), 1);
	oSpecular = float4(g_Shininess, 0, 0, 1);
	oPos = float4(In.PosVS, 1.0);
}

struct OPAQUE_VS_OUTPUT
{
	float4 Pos				: SV_Position;
	float2 Tex0				: TEXCOORD0;
	float4 Color			: COLOR0;
	float4 ShadowCoord		: TEXCOORD3;
	float3 ViewVS			: TEXCOORD4;
    float2 Tex1             : TEXCOORD5;
};

OPAQUE_VS_OUTPUT OpaqueVS( VS_INPUT In )
{
    OPAQUE_VS_OUTPUT Output;
	float4 PosWS = TransformPosWS(In);
	Output.Pos = mul(PosWS, g_ViewProj);
	Output.Tex0 = TransformUV(In) * g_TextureScale;
	Output.Color = TransformColor(In);
	Output.ShadowCoord = mul(PosWS, g_SkyLightViewProj);
	Output.ViewVS = mul(g_Eye - PosWS.xyz, (float3x3)g_View); // ! dont normalize here
    Output.Tex1 = TransformUV(In);
    return Output;    
}

float4 OpaquePS( OPAQUE_VS_OUTPUT In ) : COLOR0
{ 
	float3 SkyLightDir = normalize(float3(g_SkyLightView[0][2], g_SkyLightView[1][2], g_SkyLightView[2][2]));
	float3 SkyLightDirVS = mul(SkyLightDir, (float3x3)g_View);
	float LightAmount = GetLigthAmount(In.ShadowCoord);
	float3 Normal = tex2D(NormalRTSampler, (In.Pos.xy + 0.5f) / g_ScreenDim).xyz;
	float3 SkyDiffuse = saturate(dot(Normal, SkyLightDirVS) * LightAmount) * g_SkyLightColor.xyz;
	float3 Ref = Reflection(Normal, In.ViewVS);
	float SkySpecular = pow(saturate(dot(Ref, SkyLightDirVS) * LightAmount), g_Shininess) * g_SkyLightColor.w;
	float4 Diffuse = float4(0,0,0,0);
	float3 Specular = float3(0,0,0);
    float4 Color = tex2D(WeightTexSampler, In.Tex1);
	if (Color.r >= 0.004)
	{
		Diffuse += tex2D(DiffuseTextureSampler0, In.Tex0) * Color.r;
		Specular += tex2D(SpecularTextureSampler0, In.Tex0).xyz * Color.r;
	}
	if (Color.g >= 0.004)
	{
		Diffuse += tex2D(DiffuseTextureSampler1, In.Tex0) * Color.g;
		Specular += tex2D(SpecularTextureSampler1, In.Tex0).xyz * Color.g;
	}
	if (Color.b >= 0.004)
	{
		Diffuse += tex2D(DiffuseTextureSampler2, In.Tex0) * Color.b;
		Specular += tex2D(SpecularTextureSampler2, In.Tex0).xyz * Color.b;
	}
	if (Color.a >= 0.004)
	{
		Diffuse += tex2D(DiffuseTextureSampler3, In.Tex0) * Color.a;
		Specular += tex2D(SpecularTextureSampler3, In.Tex0).xyz * Color.a;
	}
	float4 ScreenLight = tex2D(LightRTSampler, (In.Pos.xy + 0.5f) / g_ScreenDim);
	float3 Final = Diffuse.xyz * (ScreenLight.xyz + SkyDiffuse) + Specular * (ScreenLight.w + SkySpecular);
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
