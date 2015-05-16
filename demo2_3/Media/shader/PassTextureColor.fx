
struct VS_OUTPUT
{
	float4 Pos				: POSITION;
	float2 Tex0				: TEXCOORD0;
	float4 Pos2				: TEXCOORD2;
	float4 PosLS			: TEXCOORD5;
#ifdef TEXTURE_TYPE_SPECULAR
	float3 View				: TEXCOORD3;
#endif
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

VS_OUTPUT RenderSceneVS( VS_INPUT In )
{
    VS_OUTPUT Output;
	float4 PosWS = TransformPosWS(In);
	Output.Pos = mul(PosWS, g_ViewProj);
	Output.Tex0 = TransformUV(In);
	Output.Pos2 = Output.Pos;
	Output.PosLS = mul(TransformPosWS(In), g_SkyLightViewProj);
#ifdef TEXTURE_TYPE_SPECULAR
	Output.View = g_Eye - PosWS;
#endif
    return Output;    
}

float4 RenderScenePS( VS_OUTPUT In ) : COLOR0
{ 
	float2 DiffuseTex = In.Pos2.xy / In.Pos2.w * 0.5 + 0.5;
	DiffuseTex.y = 1 - DiffuseTex.y;
	DiffuseTex = DiffuseTex + float2(0.5, 0.5) / g_ScreenDim.x;
	float3 Normal = tex2D(NormalRTSampler, DiffuseTex);
	float3 DiffuseSky = saturate(-dot(Normal, g_SkyLightDir) * GetLigthAmount(In.PosLS)) * g_SkyLightColor.xyz;
	float4 Diffuse = tex2D(DiffuseRTSampler, DiffuseTex);
	Diffuse.xyz += DiffuseSky;
	Diffuse.xyz *= tex2D(MeshTextureSampler, In.Tex0).xyz;
#ifdef TEXTURE_TYPE_SPECULAR
	float3 Ref = Reflection(Normal.xyz, In.View);
	float SpecularSky = pow(saturate(dot(Ref, -g_SkyLightDir)), 5) * g_SkyLightColor.w;
	float Specular = tex2D(SpecularTextureSampler, In.Tex0).x * (Diffuse.w + SpecularSky);
	Diffuse.xyz += Specular;
#endif
    return float4(Diffuse.xyz, 1);
}
