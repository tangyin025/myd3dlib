float3 _BaseColorTop = float3(0.5,1,0.19);
float3 _BaseColorBottom = float3(0.12,0.63,0.15);
texture _NoiseMap:MaterialParameter<string path="texture/noiseTexture.png";>;

sampler sampler_NoiseMap = sampler_state
{
    Texture = <_NoiseMap>;
    MipFilter = LINEAR;
    MinFilter = LINEAR;
    MagFilter = LINEAR;
};

struct NORMAL_VS_OUTPUT
{
	float4 Pos				: SV_Position;
	float3 Normal			: NORMAL;
	float3 PosVS			: TEXCOORD0;
};

float4 TransformPosWS2(VS_INPUT In)
{
	float4 Pos = In.Pos;
	float2 uv = Pos.xz / 10 + g_Time * float2(0.1, 0.1);
	float waveSample = tex2Dlod(sampler_NoiseMap, float4(uv, 0, 0));
	Pos.x += sin(waveSample) * (1 - In.Tex0.y);
	Pos.z += sin(waveSample) * (1 - In.Tex0.y);
	Pos.xyz += mul(In.PosInst, g_World);
	return Pos;
}

NORMAL_VS_OUTPUT NormalVS( VS_INPUT In )
{
	NORMAL_VS_OUTPUT Output;
	float4 PosWS = TransformPosWS2(In);
	Output.Pos = mul(PosWS, g_ViewProj);
	Output.Normal = mul(TransformNormal(In), (float3x3)g_View);
	Output.PosVS = mul(PosWS, g_View).xyz;
	return Output;
}

void NormalPS( 	NORMAL_VS_OUTPUT In,
				out float4 oNormal : COLOR0,
				out float4 oSpecular : COLOR1,
				out float4 oPos : COLOR2 )
{
	oNormal = float4(In.Normal, 1.0);
	oSpecular = float4(1, 0, 0, 1);
	oPos = float4(In.PosVS, 1.0);
}

struct OPAQUE_VS_OUTPUT
{
	float4 Pos				: SV_Position;
	float4 Color			: COLOR0;
	float4 ShadowCoord		: TEXCOORD1;
	float3 ViewVS			: TEXCOORD2;
};

OPAQUE_VS_OUTPUT OpaqueVS( VS_INPUT In )
{
    OPAQUE_VS_OUTPUT Output;
	float4 PosWS = TransformPosWS2(In);
	Output.Pos = mul(PosWS, g_ViewProj);
	Output.Color = float4(lerp(_BaseColorTop, _BaseColorBottom, In.Tex0.y), 1.0);
	// float4 uv = float4(PosWS.xz / 16, 0, 0);
	// uv.xy += g_Time * float2(0.1,0.2);
	// Output.Color = tex2Dlod(sampler_NoiseMap, uv);
	Output.ShadowCoord = mul(PosWS, g_SkyLightViewProj);
	Output.ViewVS = mul(g_Eye - PosWS.xyz, (float3x3)g_View); // ! dont normalize here
	return Output;
}

float4 OpaquePS( OPAQUE_VS_OUTPUT In ) : COLOR0
{ 
	float3 SkyLightDir = normalize(float3(g_SkyLightView[0][2], g_SkyLightView[1][2], g_SkyLightView[2][2]));
	float3 SkyLightDirVS = mul(SkyLightDir, (float3x3)g_View);
	float LightAmount = GetLigthAmount(In.ShadowCoord);
	float3 NormalVS = tex2D(NormalRTSampler, (In.Pos.xy + 0.5f) / g_ScreenDim).xyz;
	float3 SkyDiffuse = saturate(dot(NormalVS, SkyLightDirVS) * LightAmount) * g_SkyLightColor.xyz;
	float3 Ref = Reflection(NormalVS, In.ViewVS);
	float SkySpecular = pow(saturate(dot(Ref, SkyLightDirVS) * LightAmount), 1.0) * g_SkyLightColor.w;
	float4 ScreenLight = tex2D(LightRTSampler, (In.Pos.xy + 0.5f) / g_ScreenDim);
	float3 Final = In.Color.xyz * In.Color.xyz * (ScreenLight.xyz + SkyDiffuse) + 0.1 * (ScreenLight.w + SkySpecular);
	return float4(Final, 1);
}

technique RenderScene
{
    pass PassTypeShadow
    {          
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
