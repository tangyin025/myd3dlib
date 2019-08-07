
texture g_DiffuseTexture:MaterialParameter<string Initialize="texture/Checker.bmp";>;
texture g_SpecularTexture:MaterialParameter<string Initialize="texture/White.dds";>;

sampler DiffuseTextureSampler = sampler_state
{
    Texture = <g_DiffuseTexture>;
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
	float3 Normal			: TEXCOORD1;
	float3 ViewPos			: TEXCOORD4;
};

NORMAL_VS_OUTPUT NormalVS( VS_INPUT In )
{
	NORMAL_VS_OUTPUT Output;
	float4 PosWS = TransformPosWS(In);
	Output.Pos = mul(PosWS, g_ViewProj);
	Output.Tex0 = TransformUV(In);
	float3 Normal = TransformNormal(In);
	float3 ViewDir = g_Eye - PosWS.xyz;
	if (dot(ViewDir, Normal) < 0)
	{
		Output.Normal = mul(-Normal, (float3x3)g_View);
	}
	else
	{
		Output.Normal = mul(Normal, (float3x3)g_View);
	}
	Output.ViewPos = mul(PosWS, g_View);
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
	float4 Pos				: POSITION;
	float2 Tex0				: TEXCOORD0;
	float4 PosScreen		: TEXCOORD2;
	float4 PosShadow		: TEXCOORD5;
	float3 ViewDir			: TEXCOORD3;
};

COLOR_VS_OUTPUT OpaqueVS( VS_INPUT In )
{
    COLOR_VS_OUTPUT Output;
	float4 PosWS = TransformPosWS(In);
	Output.Pos = mul(PosWS, g_ViewProj);
	Output.Tex0 = TransformUV(In);
	Output.PosScreen = Output.Pos;
	Output.PosShadow = mul(PosWS, g_SkyLightViewProj);
	Output.ViewDir = mul(g_Eye - PosWS, (float3x3)g_View);
    return Output;    
}

float4 OpaquePS( COLOR_VS_OUTPUT In ) : COLOR0
{ 
	float3 SkyLightDir = normalize(float3(g_SkyLightViewProj[0][2],g_SkyLightViewProj[1][2],g_SkyLightViewProj[2][2]));
	float3 ViewSkyLightDir = mul(SkyLightDir, g_View);
	float2 ScreenTex = In.PosScreen.xy / In.PosScreen.w * 0.5 + 0.5;
	ScreenTex.y = 1 - ScreenTex.y;
	ScreenTex = ScreenTex + float2(0.5, 0.5) / g_ScreenDim.x;
	float LightAmount = GetLigthAmount(In.PosShadow);
	float3 Normal = tex2D(NormalRTSampler, ScreenTex);
	float3 SkyDiffuse = saturate(-dot(Normal, ViewSkyLightDir) * LightAmount) * g_SkyLightColor.xyz;
	float3 Ref = Reflection(Normal.xyz, In.ViewDir);
	float SkySpecular = pow(saturate(dot(Ref, -ViewSkyLightDir) * LightAmount), 5) * g_SkyLightColor.w;
	float4 Diffuse = tex2D(DiffuseTextureSampler, In.Tex0);
	Diffuse *= tex2D(LightRTSampler, ScreenTex) + float4(SkyDiffuse, SkySpecular);
	float Specular = tex2D(SpecularTextureSampler, In.Tex0).x * Diffuse.w;
	Diffuse.xyz += Specular;
    return float4(Diffuse.xyz, 1);
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
    pass PassTypeOpaque
    {          
        VertexShader = compile vs_3_0 OpaqueVS();
        PixelShader  = compile ps_3_0 OpaquePS(); 
    }
    pass PassTypeTransparent
    {          
        VertexShader = compile vs_3_0 OpaqueVS();
        PixelShader  = compile ps_3_0 OpaquePS(); 
    }
}
