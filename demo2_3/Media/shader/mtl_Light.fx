
struct LIGHT_VS_OUTPUT
{
	float4 Pos				: SV_Position;
	float4 Color			: COLOR0;
	float4 LightVS			: TEXCOORD0;
	float3 EyeVS			: TEXCOORD1;
};

LIGHT_VS_OUTPUT LightVS( VS_INPUT In )
{
	LIGHT_VS_OUTPUT Output;
	float4 PosWS = TransformPosWS(In);
	Output.Pos = mul(PosWS, g_ViewProj);
	Output.Color = TransformColor(In);
	float4 LightWS = TransformLightWS(In);
	Output.LightVS.xyz = mul(float4(LightWS.xyz, 1.0), g_View).xyz;
	Output.LightVS.w = LightWS.w;
	Output.EyeVS = mul(float4(g_Eye, 1.0), g_View).xyz;
	return Output;
}

float4 LightPS( LIGHT_VS_OUTPUT In ) : COLOR0
{ 
	float4 NormalVS = tex2D(NormalRTSampler, (In.Pos.xy + 0.5f) / g_ScreenDim);
	float4 PosVS = tex2D(PositionRTSampler, (In.Pos.xy + 0.5f) / g_ScreenDim);
	float3 LightDir = In.LightVS.xyz - PosVS.xyz;
	float LightDist = length(LightDir);
	LightDir = LightDir / LightDist;
	float diffuse = saturate(dot(NormalVS.xyz, LightDir));
	float3 ViewVS = In.EyeVS - PosVS.xyz;
	float3 Ref = Reflection(NormalVS.xyz, ViewVS);
	float Specular = pow(saturate(dot(Ref, LightDir)), NormalVS.w);
	return float4(In.Color.xyz * diffuse, In.Color.w * Specular) * saturate(1 - LightDist / In.LightVS.w);
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
		VertexShader = compile vs_3_0 LightVS();
		PixelShader  = compile ps_3_0 LightPS();
    }
	pass PassTypeBackground
	{
	}
    pass PassTypeOpaque
    {          
    }
    pass PassTypeLight
    {          
    }
}
