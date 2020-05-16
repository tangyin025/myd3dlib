
struct LIGHT_VS_OUTPUT
{
	float4 Pos				: POSITION;
	float4 Color			: COLOR0;
	float4 ScreenPos		: TEXCOORD0;
	float2 ScreenTex		: TEXCOORD1;
	float4 Light			: TEXCOORD2;
	float3 Eye				: TEXCOORD3;
};

LIGHT_VS_OUTPUT LightVS( VS_INPUT In )
{
	LIGHT_VS_OUTPUT Output;
	float4 PosWS = TransformPosWS(In);
	Output.Pos = mul(PosWS, g_ViewProj);
	Output.Color = TransformColor(In);
	Output.ScreenPos = Output.Pos;
	Output.ScreenTex.x = Output.Pos.x * 0.5 + Output.Pos.w * 0.5 + Output.Pos.w * 0.5 / g_ScreenDim.x;
	Output.ScreenTex.y = Output.Pos.w - Output.Pos.y * 0.5 - 0.5 * Output.Pos.w + Output.Pos.w * 0.5 / g_ScreenDim.y;
	Output.Light = TransformLightWS(In);
	Output.Light.xyz = mul(float4(Output.Light.xyz, 1.0), g_View).xyz;
	Output.Eye = mul(float4(g_Eye, 1.0), g_View).xyz;
	return Output;
}

float4 LightPS( LIGHT_VS_OUTPUT In ) : COLOR0
{ 
	float4 Normal = tex2D(NormalRTSampler, In.ScreenTex / In.ScreenPos.w);
	float4 ViewPos = tex2D(PositionRTSampler, In.ScreenTex / In.ScreenPos.w);
	float3 LightVec = In.Light.xyz - ViewPos.xyz;
	float LightDist = length(LightVec);
	LightVec = LightVec / LightDist;
	float diffuse = saturate(dot(Normal.xyz, LightVec));
	float3 View = In.Eye - ViewPos.xyz;
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
    pass PassTypeOpaque
    {          
    }
    pass PassTypeLight
    {          
    }
}
