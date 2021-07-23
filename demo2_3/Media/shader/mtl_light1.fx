
struct LIGHT_VS_OUTPUT
{
	float4 Pos				: SV_Position;
	float4 Color			: COLOR0;
	float4 Light			: TEXCOORD0;
	float3 Eye				: TEXCOORD1;
};

LIGHT_VS_OUTPUT LightVS( VS_INPUT In )
{
	LIGHT_VS_OUTPUT Output;
	float4 PosWS = TransformPosWS(In);
	Output.Pos = mul(PosWS, g_ViewProj);
	Output.Color = TransformColor(In);
	Output.Light = TransformLightWS(In);
	Output.Light.xyz = mul(float4(Output.Light.xyz, 1.0), g_View).xyz;
	Output.Eye = mul(float4(g_Eye, 1.0), g_View).xyz;
	return Output;
}

float4 LightPS( LIGHT_VS_OUTPUT In ) : COLOR0
{ 
	float4 Normal = tex2D(NormalRTSampler, (In.Pos.xy + 0.5f) / g_ScreenDim);
	float4 PosVS = tex2D(PositionRTSampler, (In.Pos.xy + 0.5f) / g_ScreenDim);
	float3 LightVec = In.Light.xyz - PosVS.xyz;
	float LightDist = length(LightVec);
	LightVec = LightVec / LightDist;
	float diffuse = saturate(dot(Normal.xyz, LightVec));
	float3 View = In.Eye - PosVS.xyz;
	float3 Ref = Reflection(Normal.xyz, View);
	float Specular = pow(saturate(dot(Ref, LightVec)), Normal.w);
	return float4(In.Color.xyz * diffuse, In.Color.w * Specular) * saturate(1 - LightDist / In.Light.w);
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
