
texture g_DiffuseTexture:MaterialParameter<string Initialize="texture/Checker.bmp";>;
texture g_NormalTexture:MaterialParameter<string Initialize="texture/Normal.dds";>;
texture g_SpecularTexture:MaterialParameter<string Initialize="texture/White.dds";>;
float g_SpecularExp:MaterialParameter = 25;
float g_AlphaMask:MaterialParameter = 0.5;

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
	float2 Tex1				: TEXCOORD1;
};

SHADOW_VS_OUTPUT ShadowVS( VS_INPUT In )
{
    SHADOW_VS_OUTPUT Output;
	Output.Pos = TransformPosShadow(In);
	Output.Tex0 = Output.Pos.zw;
	Output.Tex1 = TransformUV(In);
    return Output;    
}

float4 ShadowPS( SHADOW_VS_OUTPUT In ) : COLOR0
{ 
	float4 Diffuse = tex2D(DiffuseTextureSampler, In.Tex1);
	clip(Diffuse.a - g_AlphaMask);
    return In.Tex0.x / In.Tex0.y;
}

struct NORMAL_VS_OUTPUT
{
	float4 Pos				: POSITION;
	float2 Tex0				: TEXCOORD0;
	float3 Normal			: NORMAL;
	float3 Tangent			: TANGENT;
	float3 Binormal			: BINORMAL;
	float3 ViewPos			: TEXCOORD1;
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
	Output.ViewPos = mul(PosWS, g_View).xyz;
	return Output;
}

void NormalPS( 	NORMAL_VS_OUTPUT In,
				out float4 oNormal : COLOR0,
				out float4 oPos : COLOR1 )
{
	float4 Diffuse = tex2D(DiffuseTextureSampler, In.Tex0);
	clip(Diffuse.a - g_AlphaMask);
	float3x3 m = float3x3(In.Tangent, In.Binormal, In.Normal);
	float3 NormalTS = tex2D(NormalTextureSampler, In.Tex0).xyz * 2 - 1;
	oNormal = float4(mul(NormalTS, m), 1.0);
	oPos = float4(In.ViewPos, 1.0);
}

struct COLOR_VS_OUTPUT
{
	float4 Pos				: POSITION;
	float2 Tex0				: TEXCOORD0;
	float4 ScreenPos		: TEXCOORD1;
	float2 ScreenTex		: TEXCOORD2;
	float4 ShadowPos		: TEXCOORD3;
	float3 ViewDir			: TEXCOORD4;
};

COLOR_VS_OUTPUT OpaqueVS( VS_INPUT In )
{
    COLOR_VS_OUTPUT Output;
	float4 PosWS = TransformPosWS(In);
	Output.Pos = mul(PosWS, g_ViewProj);
	Output.Tex0 = TransformUV(In);
	Output.ScreenPos = Output.Pos;
	Output.ScreenTex.x = Output.Pos.x * 0.5 + Output.Pos.w * 0.5 + Output.Pos.w * 0.5 / g_ScreenDim.x;
	Output.ScreenTex.y = Output.Pos.w - Output.Pos.y * 0.5 - 0.5 * Output.Pos.w + Output.Pos.w * 0.5 / g_ScreenDim.y;
	Output.ShadowPos = mul(PosWS, g_SkyLightViewProj);
	Output.ViewDir = mul(g_Eye - PosWS.xyz, (float3x3)g_View); // ! dont normalize here
    return Output;    
}

float4 OpaquePS( COLOR_VS_OUTPUT In ) : COLOR0
{ 
	float4 Diffuse = tex2D(DiffuseTextureSampler, In.Tex0);
	clip(Diffuse.a - g_AlphaMask);
	float3 SkyLightDir = normalize(float3(g_SkyLightView[0][2], g_SkyLightView[1][2], g_SkyLightView[2][2]));
	float3 ViewSkyLightDir = mul(SkyLightDir, (float3x3)g_View);
	float LightAmount = GetLigthAmount(In.ShadowPos);
	float3 Normal = tex2D(NormalRTSampler, In.ScreenTex / In.ScreenPos.w).xyz;
	float3 SkyDiffuse = saturate(dot(Normal, ViewSkyLightDir) * LightAmount) * g_SkyLightColor.xyz;
	float3 Ref = Reflection(Normal, In.ViewDir);
	float SkySpecular = pow(saturate(dot(Ref, ViewSkyLightDir) * LightAmount), g_SpecularExp) * g_SkyLightColor.w;
	float3 Specular = tex2D(SpecularTextureSampler, In.Tex0).xyz;
	float4 ScreenLight = tex2D(LightRTSampler, In.ScreenTex / In.ScreenPos.w);
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
