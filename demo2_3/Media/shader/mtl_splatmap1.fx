
texture g_DiffuseTexture0:MaterialParameter<string Initialize="texture/Checker.bmp";>;
texture g_DiffuseTexture1:MaterialParameter<string Initialize="texture/Checker.bmp";>;
texture g_DiffuseTexture2:MaterialParameter<string Initialize="texture/Checker.bmp";>;
texture g_DiffuseTexture3:MaterialParameter<string Initialize="texture/Checker.bmp";>;
float2 g_TextureScale:MaterialParameter = float2(1.0, 1.0);
float g_SpecularStrength:MaterialParameter = 0.5;

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
	float3 Normal			: TEXCOORD1;
	// float3 Tangent			: TEXCOORD2;
	// float3 Binormal			: TEXCOORD3;
	float3 ViewPos			: TEXCOORD4;
};

NORMAL_VS_OUTPUT NormalVS( VS_INPUT In )
{
	NORMAL_VS_OUTPUT Output;
	float4 PosWS = TransformPosWS(In);
	Output.Pos = mul(PosWS, g_ViewProj);
	Output.Tex0 = TransformUV(In) * g_TextureScale;
	Output.Normal = mul(TransformNormal(In), (float3x3)g_View);
	// Output.Tangent = mul(TransformTangent(In), (float3x3)g_View);
	// Output.Binormal = cross(Output.Normal, Output.Tangent);
	Output.ViewPos = mul(PosWS, g_View);
	return Output;
}

void NormalPS( 	NORMAL_VS_OUTPUT In,
				out float4 oNormal : COLOR0,
				out float4 oPos : COLOR1 )
{
	// float3x3 m = float3x3(In.Tangent, In.Binormal, In.Normal);
	// float3 NormalTS = tex2D(NormalTextureSampler, In.Tex0).xyz * 2 - 1;
	// oNormal = float4(mul(NormalTS, m), 1.0);
	oNormal = float4(In.Normal, 1.0);
	oPos = float4(In.ViewPos, 1.0);
}

struct COLOR_VS_OUTPUT
{
	float4 Pos				: POSITION;
	float2 Tex0				: TEXCOORD0;
	float3 Normal			: TEXCOORD1;
	float4 Color			: COLOR0;
	float4 PosScreen		: TEXCOORD2;
	float4 PosShadow		: TEXCOORD5;
	float3 ViewDir			: TEXCOORD3;
};

COLOR_VS_OUTPUT OpaqueVS( VS_INPUT In )
{
    COLOR_VS_OUTPUT Output;
	float4 PosWS = TransformPosWS(In);
	Output.Pos = mul(PosWS, g_ViewProj);
	Output.Tex0 = TransformUV(In) * g_TextureScale;
	Output.Normal = mul(TransformNormal(In), (float3x3)g_View);
	Output.Color = TransformColor(In);
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
	// float3 Normal = tex2D(NormalRTSampler, ScreenTex);
	float3 SkyDiffuse = saturate(-dot(In.Normal, ViewSkyLightDir) * LightAmount) * g_SkyLightColor.xyz;
	float3 Ref = Reflection(In.Normal.xyz, In.ViewDir);
	float SkySpecular = pow(saturate(dot(Ref, -ViewSkyLightDir) * LightAmount), 5) * g_SkyLightColor.w;
	float4 Diffuse = float4(0,0,0,0);
	Diffuse += tex2D(DiffuseTextureSampler0, In.Tex0) * In.Color.a;
	Diffuse += tex2D(DiffuseTextureSampler1, In.Tex0) * In.Color.r;
	Diffuse += tex2D(DiffuseTextureSampler2, In.Tex0) * In.Color.g;
	Diffuse += tex2D(DiffuseTextureSampler3, In.Tex0) * In.Color.b;
	Diffuse *= tex2D(LightRTSampler, ScreenTex) + float4(SkyDiffuse, SkySpecular);
	float Specular = g_SpecularStrength * Diffuse.w;
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
    }
}
