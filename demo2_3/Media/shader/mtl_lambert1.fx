
float g_FresExp:MaterialParameter = 3.0;
float g_ReflStrength:MaterialParameter = 3.4;
float g_SpecularPower:MaterialParameter = 5;
texture g_DiffuseTexture:MaterialParameter<string Initialize="texture/Checker.bmp";>;
texture g_NormalTexture:MaterialParameter<string Initialize="texture/Normal.dds";>;
texture g_SpecularTexture:MaterialParameter<string Initialize="texture/White.dds";>;

sampler DiffuseTextureSampler = sampler_state
{
    Texture = <g_DiffuseTexture>;
    MipFilter = LINEAR;
    MinFilter = LINEAR;
    MagFilter = LINEAR;
    ADDRESSU = WRAP;
    ADDRESSV = WRAP;
};

sampler NormalTextureSampler = sampler_state
{
	Texture = <g_NormalTexture>;
	MipFilter = LINEAR;
	MinFilter = LINEAR;
	MagFilter = LINEAR;
    ADDRESSU = WRAP;
    ADDRESSV = WRAP;
};

sampler SpecularTextureSampler = sampler_state
{
	Texture = <g_SpecularTexture>;
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
	float2 Tex0				: TEXCOORD0;
	float3 Normal			: TEXCOORD1;
	float3 Tangent			: TEXCOORD2;
	float3 Binormal			: TEXCOORD3;
	float3 ViewPos			: TEXCOORD4;
};

NORMAL_VS_OUTPUT NormalVS( VS_INPUT In )
{
	NORMAL_VS_OUTPUT Output;
	float4 PosWS = TransformPosWS(In);
	Output.Pos = mul(PosWS, g_ViewProj);
	Output.Tex0 = TransformUV(In);
	Output.Normal = mul(TransformNormal(In), (float3x3)g_View);
	Output.Tangent = mul(TransformTangent(In), (float3x3)g_View);
	Output.Binormal = cross(Output.Normal, Output.Tangent);
	Output.ViewPos = mul(PosWS, g_View);
	return Output;
}

void NormalPS( 	NORMAL_VS_OUTPUT In,
				out float4 oNormal : COLOR0,
				out float4 oPos : COLOR1 )
{
	float3x3 m = float3x3(In.Tangent, In.Binormal, In.Normal);
	float3 NormalTS = tex2D(NormalTextureSampler, In.Tex0).xyz * 2 - 1;
	oNormal = float4(mul(NormalTS, m), 1.0);
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
	Output.ViewDir = normalize(mul(g_Eye - PosWS, (float3x3)g_View));
    return Output;    
}

float4 OpaquePS( COLOR_VS_OUTPUT In ) : COLOR0
{ 
	float3 SkyLightDir = normalize(float3(g_SkyLightViewProj[0][2],g_SkyLightViewProj[1][2],g_SkyLightViewProj[2][2]));
	float3 ViewSkyLightDir = mul(SkyLightDir, g_View);
	float2 ScreenTex = In.PosScreen.xy / In.PosScreen.w * 0.5 + 0.5;
	ScreenTex.y = 1 - ScreenTex.y;
	ScreenTex = ScreenTex + float2(0.5, 0.5) / g_ScreenDim.x;
	float3 Normal = tex2D(NormalRTSampler, ScreenTex);
	float4 Ambient=tex2D(LightRTSampler, ScreenTex);
	float fres=Fresnel(Normal,In.ViewDir,g_FresExp,g_ReflStrength);
	float4 TexDiffuse = tex2D(DiffuseTextureSampler, In.Tex0);
	float3 Diffuse = saturate(dot(Normal, -ViewSkyLightDir)) * g_SkyLightColor.xyz;
	float LightAmount = GetLigthAmount(In.PosShadow);
	float3 Ref = Reflection(Normal, In.ViewDir);
	float4 TexSpecular = tex2D(SpecularTextureSampler, In.Tex0);
	float Specular = pow(saturate(dot(Ref, -ViewSkyLightDir)), g_SpecularPower) * g_SkyLightColor.w;
	float3 Final = TexDiffuse.xyz * LightAmount * (Diffuse + Specular) + TexDiffuse.xyz * (Ambient.xyz + Ambient.w * fres);
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
    }
}
