
struct LIGHT_VS_OUTPUT
{
	float4 Pos				: POSITION;
	float4 Pos2				: TEXCOORD0;
	float4 Light			: TEXCOORD1;
	float4 Color			: COLOR0;
};

LIGHT_VS_OUTPUT LightVS( VS_INPUT In )
{
	LIGHT_VS_OUTPUT Output;
	Output.Pos = TransformPos(In);
	Output.Pos2 = Output.Pos;
	Output.Light = TransformLight(In);
	Output.Color = TransformColor(In);
	return Output;
}

float4 LightPS( LIGHT_VS_OUTPUT In ) : COLOR0
{ 
	float2 NormalTex = In.Pos2.xy / In.Pos2.w * 0.5 + 0.5;
	NormalTex.y = 1 - NormalTex.y;
	NormalTex = NormalTex + float2(0.5, 0.5) / g_ScreenDim;
	float4 Normal = tex2D(NormalRTSampler, NormalTex);
	float z = Normal.w;
	float4 PosWS = mul(float4(In.Pos2.x, In.Pos2.y, z * In.Pos2.w, In.Pos2.w), g_InvViewProj);
	PosWS.xyz = PosWS.xyz / PosWS.www;
	PosWS.w = 1;
	float3 LightVec = In.Light.xyz - PosWS.xyz;
	float LightLen = length(LightVec);
	LightVec = LightVec / LightLen;
	float diffuse = saturate(dot(Normal.xyz, LightVec));
	float3 View = g_Eye - PosWS.xyz;
	float3 Ref = Reflection(Normal.xyz, View);
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
