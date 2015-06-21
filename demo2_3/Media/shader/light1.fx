
struct LIGHT_VS_OUTPUT
{
	float4 Pos				: POSITION;
	float4 ScreenPos		: TEXCOORD0;
	float4 Light			: TEXCOORD1;
	float3 Eye				: TEXCOORD2;
	float4 Color			: COLOR0;
};

LIGHT_VS_OUTPUT LightVS( VS_INPUT In )
{
	LIGHT_VS_OUTPUT Output;
	float4 PosWS = TransformPosWS(In);
	Output.Pos = mul(PosWS, g_ViewProj);
	Output.ScreenPos = Output.Pos;
	Output.Light = TransformLight(In);
	Output.Light.xyz = mul(float4(Output.Light.xyz, 1.0), g_View);
	Output.Eye = mul(float4(g_Eye, 1.0), g_View);
	Output.Color = TransformColor(In);
	return Output;
}

float4 LightPS( LIGHT_VS_OUTPUT In ) : COLOR0
{ 
	float2 NormalTex = In.ScreenPos.xy / In.ScreenPos.w * 0.5 + 0.5;
	NormalTex.y = 1 - NormalTex.y;
	NormalTex = NormalTex + float2(0.5, 0.5) / g_ScreenDim;
	float3 Normal = tex2D(NormalRTSampler, NormalTex).xyz;
	float3 ViewPos = tex2D(PositionRTSampler, NormalTex).xyz;
	float3 LightVec = In.Light.xyz - ViewPos;
	float LightLen = length(LightVec);
	clip(In.Light.w - LightLen);
	LightVec = LightVec / LightLen;
	float diffuse = saturate(dot(Normal, LightVec));
	float3 View = In.Eye - ViewPos;
	float3 Ref = Reflection(Normal, View);
	float specular = pow(saturate(dot(Ref, LightVec)), 5);
	return float4(In.Color.xyz * diffuse, In.Color.w * specular) * saturate(1 - LightLen / In.Light.w);
}

technique RenderScene
{
    pass PassTypeShadow
    {          
    }
    pass PassTypeNormal
    {          
    }
    pass PassTypeLight
    {          
		VertexShader = compile vs_2_0 LightVS();
		PixelShader  = compile ps_2_0 LightPS();
    }
    pass PassTypeOpaque
    {          
    }
    pass PassTypeLight
    {          
    }
}
