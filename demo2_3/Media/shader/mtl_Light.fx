#include "CommonVert.hlsl"

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
	float3 NormalVS = tex2D(NormalRTSampler, (In.Pos.xy + 0.5f) / g_ScreenDim).xyz;
	float3 SpecularVS = tex2D(SpecularRTSampler, (In.Pos.xy + 0.5f) / g_ScreenDim).xyz;
	float4 PosVS = tex2D(PositionRTSampler, (In.Pos.xy + 0.5f) / g_ScreenDim);
	float3 LightDir = In.LightVS.xyz - PosVS.xyz;
	float LightDist = length(LightDir);
	LightDir = LightDir / LightDist;
    float LightAmount = saturate(dot(NormalVS, LightDir));
    float3 ViewVS = normalize(PosVS.xyz - In.EyeVS);
    float3 Ref = reflect(ViewVS, NormalVS);
    float Specular = DistributionGGX(LightDir, Ref, SpecularVS.r) * In.Color.w * SpecularVS.g * LightAmount;
    return float4(In.Color.xyz * LightAmount, Specular) * (1 - SplineInterpolate(clamp(LightDist / In.LightVS.w, 0, 1)));
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
