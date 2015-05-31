
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
	float2 Depth			: TEXCOORD4;
};

NORMAL_VS_OUTPUT NormalVS( VS_INPUT In )
{
	NORMAL_VS_OUTPUT Output;
	Output.Pos = TransformPos(In);
	Output.Tex0 = TransformUV(In);
	Output.Normal = TransformNormal(In);
	Output.Tangent = TransformTangent(In);
	Output.Binormal = cross(Output.Normal, Output.Tangent);
	Output.Depth = Output.Pos.zw;
	return Output;
}

float4 NormalPS( NORMAL_VS_OUTPUT In ) : COLOR0
{
	float3x3 t2w = float3x3(In.Tangent, In.Binormal, In.Normal);
	float3 NormalTS = tex2D(NormalTextureSampler, In.Tex0).xyz * 2 - 1;
	float3 Normal = mul(NormalTS, t2w);
	return float4(Normal.x, Normal.y, Normal.z, In.Depth.x / In.Depth.y);
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
			LightAmount += tex2D(ShadowRTSampler, ShadowTexC + float2(x, y) / SHADOW_MAP_SIZE) + SHADOW_EPSILON < PosLS.z / PosLS.w ? 0.0f : 1.0f;
			
	return LightAmount / 4;
}

COLOR_VS_OUTPUT ColorVS( VS_INPUT In )
{
    COLOR_VS_OUTPUT Output;
	float4 PosWS = TransformPosWS(In);
	Output.Pos = mul(PosWS, g_ViewProj);
	Output.Tex0 = TransformUV(In);
	Output.Pos2 = Output.Pos;
	Output.PosLS = mul(TransformPosWS(In), g_SkyLightViewProj);
	Output.View = g_Eye - PosWS;
    return Output;    
}

float4 ColorPS( COLOR_VS_OUTPUT In ) : COLOR0
{ 
	float2 DiffuseTex = In.Pos2.xy / In.Pos2.w * 0.5 + 0.5;
	DiffuseTex.y = 1 - DiffuseTex.y;
	DiffuseTex = DiffuseTex + float2(0.5, 0.5) / g_ScreenDim.x;
	float3 Normal = tex2D(NormalRTSampler, DiffuseTex);
	float3 DiffuseSky = saturate(-dot(Normal, g_SkyLightDir) * GetLigthAmount(In.PosLS)) * g_SkyLightColor.xyz;
	float4 Diffuse = tex2D(DiffuseRTSampler, DiffuseTex);
	Diffuse.xyz += DiffuseSky;
	Diffuse.xyz *= tex2D(MeshTextureSampler, In.Tex0).xyz;
	float3 Ref = Reflection(Normal.xyz, In.View);
	float SpecularSky = pow(saturate(dot(Ref, -g_SkyLightDir)), 5) * g_SkyLightColor.w;
	float Specular = tex2D(SpecularTextureSampler, In.Tex0).x * (Diffuse.w + SpecularSky);
	Diffuse.xyz += Specular;
    return float4(Diffuse.xyz, 1);
}

technique RenderShadow
{
    pass P0
    {          
        VertexShader = compile vs_2_0 ShadowVS();
        PixelShader  = compile ps_2_0 ShadowPS(); 
    }
}

technique RenderNormal
{
    pass P0
    {          
        VertexShader = compile vs_2_0 NormalVS();
        PixelShader  = compile ps_2_0 NormalPS(); 
    }
}

technique RenderColor
{
    pass P0
    {          
        VertexShader = compile vs_2_0 ColorVS();
        PixelShader  = compile ps_2_0 ColorPS(); 
    }
}
