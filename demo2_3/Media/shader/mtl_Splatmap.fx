
texture g_DiffuseTexture0:MaterialParameter<string Initialize="texture/Red.dds";>;
texture g_DiffuseTexture1:MaterialParameter<string Initialize="texture/Green.dds";>;
texture g_DiffuseTexture2:MaterialParameter<string Initialize="texture/Blue.dds";>;
texture g_DiffuseTexture3:MaterialParameter<string Initialize="texture/Checker.bmp";>;
float g_Shininess:MaterialParameter = 25;
float2 g_TextureScale:MaterialParameter = float2(1.0, 1.0);

sampler DiffuseTextureSampler0 = sampler_state
{
    Texture = <g_DiffuseTexture0>;
    MipFilter = LINEAR;
    MinFilter = LINEAR;
    MagFilter = LINEAR;
};

sampler DiffuseTextureSampler1 = sampler_state
{
    Texture = <g_DiffuseTexture1>;
    MipFilter = LINEAR;
    MinFilter = LINEAR;
    MagFilter = LINEAR;
};

sampler DiffuseTextureSampler2 = sampler_state
{
    Texture = <g_DiffuseTexture2>;
    MipFilter = LINEAR;
    MinFilter = LINEAR;
    MagFilter = LINEAR;
};

sampler DiffuseTextureSampler3 = sampler_state
{
    Texture = <g_DiffuseTexture3>;
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
	float2 Tex0				: TEXCOORD0;
	float3 Normal			: NORMAL;
	float3 ViewPos			: TEXCOORD1;
};

NORMAL_VS_OUTPUT NormalVS( VS_INPUT In )
{
	NORMAL_VS_OUTPUT Output;
	float4 PosWS = TransformPosWS(In);
	Output.Pos = mul(PosWS, g_ViewProj);
	Output.Tex0 = TransformUV(In) * g_TextureScale;
	Output.Normal = mul(TransformNormal(In), (float3x3)g_View);
	Output.ViewPos = mul(PosWS, g_View).xyz;
	return Output;
}

void NormalPS( 	NORMAL_VS_OUTPUT In,
				out float4 oNormal : COLOR0,
				out float4 oPos : COLOR1 )
{
	oNormal = float4(In.Normal, 1.0);
	oPos = float4(In.ViewPos, 1.0);
}

struct COLOR_VS_OUTPUT
{
	float4 Pos				: SV_Position;
	float2 Tex0				: TEXCOORD0;
	float3 Normal			: NORMAL;
	float4 Color			: COLOR0;
	float4 ShadowPos		: TEXCOORD3;
	float3 ViewDir			: TEXCOORD4;
};

COLOR_VS_OUTPUT OpaqueVS( VS_INPUT In )
{
    COLOR_VS_OUTPUT Output;
	float4 PosWS = TransformPosWS(In);
	Output.Pos = mul(PosWS, g_ViewProj);
	Output.Tex0 = TransformUV(In) * g_TextureScale;
	Output.Normal = mul(TransformNormal(In), (float3x3)g_View);
	Output.Color = TransformColor(In);
	Output.ShadowPos = mul(PosWS, g_SkyLightViewProj);
	Output.ViewDir = mul(g_Eye - PosWS.xyz, (float3x3)g_View); // ! dont normalize here
    return Output;    
}

float4 OpaquePS( COLOR_VS_OUTPUT In ) : COLOR0
{ 
	float3 SkyLightDir = normalize(float3(g_SkyLightView[0][2], g_SkyLightView[1][2], g_SkyLightView[2][2]));
	float3 ViewSkyLightDir = mul(SkyLightDir, (float3x3)g_View);
	float LightAmount = GetLigthAmount(In.ShadowPos);
	float3 SkyDiffuse = saturate(dot(In.Normal, ViewSkyLightDir) * LightAmount) * g_SkyLightColor.xyz;
	float3 Ref = Reflection(In.Normal.xyz, In.ViewDir);
	float SkySpecular = pow(saturate(dot(Ref, ViewSkyLightDir) * LightAmount), g_Shininess) * g_SkyLightColor.w;
	float4 Diffuse = float4(0,0,0,0);
	if (In.Color.r >= 0.004)
		Diffuse += tex2D(DiffuseTextureSampler0, In.Tex0) * In.Color.r;
	if (In.Color.g >= 0.004)
		Diffuse += tex2D(DiffuseTextureSampler1, In.Tex0) * In.Color.g;
	if (In.Color.b >= 0.004)
		Diffuse += tex2D(DiffuseTextureSampler2, In.Tex0) * In.Color.b;
	if (In.Color.a >= 0.004)
		Diffuse += tex2D(DiffuseTextureSampler3, In.Tex0) * In.Color.a;
	float4 ScreenLight = tex2D(LightRTSampler, (In.Pos.xy + 0.5f) / g_ScreenDim);
	float3 Final = Diffuse.xyz * (ScreenLight.xyz + SkyDiffuse) + Diffuse.a * (ScreenLight.w + SkySpecular);
    return float4(Final.xyz, 1);
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