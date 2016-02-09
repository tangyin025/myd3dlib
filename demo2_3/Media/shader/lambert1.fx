
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
	float4 Pos2				: TEXCOORD2;
	float4 PosLS			: TEXCOORD5;
	float3 View				: TEXCOORD3;
};

float GetLigthAmount(float4 PosLS)
{
	float2 ShadowTexC = PosLS.xy / PosLS.w * 0.5 + 0.5;
	ShadowTexC.y = 1.0 - ShadowTexC.y;
	
	float LightAmount = 0;
	float x, y;
	for(x = -0.0; x <= 1.0; x += 1.0)
		for(y = -0.0; y <= 1.0; y+= 1.0)
			LightAmount += tex2D(ShadowRTSampler, ShadowTexC + float2(x, y) / SHADOW_MAP_SIZE) + SHADOW_EPSILON < PosLS.z / PosLS.w ? 0.0 : 1.0;
			
	return LightAmount / 4;
}

COLOR_VS_OUTPUT OpaqueVS( VS_INPUT In )
{
    COLOR_VS_OUTPUT Output;
	float4 PosWS = TransformPosWS(In);
	Output.Pos = mul(PosWS, g_ViewProj);
	Output.Tex0 = TransformUV(In);
	Output.Pos2 = Output.Pos;
	Output.PosLS = mul(TransformPosWS(In), g_SkyLightViewProj);
	Output.View = mul(g_Eye - PosWS, (float3x3)g_View);
    return Output;    
}

float4 OpaquePS( COLOR_VS_OUTPUT In ) : COLOR0
{ 
	float3 ViewSkyLightDir = mul(g_SkyLightDir, g_View);
	float2 DiffuseTex = In.Pos2.xy / In.Pos2.w * 0.5 + 0.5;
	DiffuseTex.y = 1 - DiffuseTex.y;
	DiffuseTex = DiffuseTex + float2(0.5, 0.5) / g_ScreenDim.x;
	float LightAmount = GetLigthAmount(In.PosLS);
	float3 Normal = tex2D(NormalRTSampler, DiffuseTex);
	float3 SkyDiffuse = saturate(-dot(Normal, ViewSkyLightDir) * LightAmount) * g_SkyLightDiffuse.xyz;
	float3 Ref = Reflection(Normal.xyz, In.View);
	float SkySpecular = pow(saturate(dot(Ref, -ViewSkyLightDir) * LightAmount), 5) * g_SkyLightDiffuse.w;
	float4 Diffuse = tex2D(MeshTextureSampler, In.Tex0) * g_MeshColor;
	Diffuse *= tex2D(LightRTSampler, DiffuseTex) + float4(SkyDiffuse, SkySpecular) + g_SkyLightAmbient;
	float Specular = tex2D(SpecularTextureSampler, In.Tex0).x * Diffuse.w;
	Diffuse.xyz += Specular;
    return float4(Diffuse.xyz, 1);
}

technique RenderScene
{
    pass PassTypeShadow
    {          
        VertexShader = compile vs_2_0 ShadowVS();
        PixelShader  = compile ps_2_0 ShadowPS(); 
    }
    pass PassTypeNormal
    {          
        VertexShader = compile vs_2_0 NormalVS();
        PixelShader  = compile ps_2_0 NormalPS(); 
    }
    pass PassTypeLight
    {          
    }
    pass PassTypeOpaque
    {          
        VertexShader = compile vs_2_0 OpaqueVS();
        PixelShader  = compile ps_2_0 OpaquePS(); 
    }
    pass PassTypeTransparent
    {          
    }
}
