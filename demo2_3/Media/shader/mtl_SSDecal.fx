
texture g_DiffuseTexture:MaterialParameter<string path="texture/Checker.bmp";>;
texture g_NormalTexture:MaterialParameter<string path="texture/Normal.dds";>;
texture g_SpecularTexture:MaterialParameter<string path="texture/White.dds";>;
float g_Shininess:MaterialParameter = 25;
float4x4 g_InvWorldView:MaterialParameter<bool UseInvWorldView=true;>;

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

// struct NORMAL_VS_OUTPUT
// {
// 	float4 Pos				: SV_Position;
// 	float4 Color			: COLOR0;
// 	float2 Tex0				: TEXCOORD0;
// 	float3 Normal			: NORMAL;
// 	float3 Tangent			: TEXCOORD1;
// 	float3 Binormal			: TEXCOORD2;
// 	float3 PosVS			: TEXCOORD3;
// };

// NORMAL_VS_OUTPUT NormalVS( VS_INPUT In )
// {
// 	NORMAL_VS_OUTPUT Output;
// 	float4 PosWS = TransformPosWS(In);
// 	Output.Pos = mul(PosWS, g_ViewProj);
// 	Output.Color = TransformColor(In);
// 	Output.Tex0 = TransformUV(In);
// 	Output.Normal = mul(TransformNormal(In), (float3x3)g_View);
// 	Output.Tangent = mul(TransformTangent(In), (float3x3)g_View);
// 	Output.Binormal = cross(Output.Normal, Output.Tangent);
// 	Output.PosVS = mul(PosWS, g_View).xyz;
// 	return Output;
// }

// void NormalPS( 	NORMAL_VS_OUTPUT In,
// 				out float4 oNormal : COLOR0,
// 				out float4 oPos : COLOR1 )
// {
// 	// 这里无法取出PositionRTSampler
// 	float4 PosVS = tex2D(PositionRTSampler, (In.Pos.xy + 0.5f) / g_ScreenDim);
// 	float4 Pos = mul(PosVS, g_InvWorldView);
// 	clip(float3(0.5,0.5,0.5)-abs(Pos));

// 	float3x3 m = float3x3(In.Tangent, In.Binormal, In.Normal);
// 	float3 NormalTS = tex2D(NormalTextureSampler, Pos.xz + 0.5).xyz * 2 - 1;
// 	oNormal = float4(mul(NormalTS, m), g_Shininess);
// 	oPos = float4(In.PosVS, 1.0);
// }

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
	float4 PosVS = tex2D(PositionRTSampler, (In.Pos.xy + 0.5f) / g_ScreenDim);
	float4 Pos = mul(PosVS, g_InvWorldView);
	clip(float3(0.5,0.5,0.5)-abs(Pos));

	float3 SkyLightDir = normalize(float3(g_SkyLightView[0][2], g_SkyLightView[1][2], g_SkyLightView[2][2]));
	float3 SkyLightDirVS = mul(SkyLightDir, (float3x3)g_View);
	float LightAmount = GetLigthAmount(In.ShadowCoord);
	float3 NormalVS = tex2D(NormalRTSampler, (In.Pos.xy + 0.5f) / g_ScreenDim).xyz;
	float3 SkyDiffuse = saturate(dot(NormalVS, SkyLightDirVS) * LightAmount) * g_SkyLightColor.xyz;
	float3 Ref = Reflection(NormalVS, In.ViewVS);
	float SkySpecular = pow(saturate(dot(Ref, SkyLightDirVS) * LightAmount), g_Shininess) * g_SkyLightColor.w;
	float4 Diffuse = tex2D(DiffuseTextureSampler, Pos.xz + 0.5);
	float3 Specular = tex2D(SpecularTextureSampler, Pos.xz + 0.5).xyz;
	float4 ScreenLight = tex2D(LightRTSampler, (In.Pos.xy + 0.5f) / g_ScreenDim);
	float3 Final = Diffuse.xyz * In.Color.xyz * (ScreenLight.xyz + SkyDiffuse) + Specular * (ScreenLight.w + SkySpecular);
    return float4(Final, In.Color.w * Diffuse.a);
}

technique RenderScene
{
    pass PassTypeShadow
    {          
    }
    pass PassTypeNormal
    {          
        // VertexShader = compile vs_3_0 NormalVS();
        // PixelShader  = compile ps_3_0 NormalPS(); 
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
