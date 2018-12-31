
struct VS_INPUT
{
	float2 Tex0  			: TEXCOORD0;
//#ifdef INSTANCE
	float4 Pos				: POSITION;
	float3 Velocity			: NORMAL;
	float4 Color			: TEXCOORD1;
	float4 Tex2				: TEXCOORD2; // size_x, size_y, angle, time
//#endif
};

float4 TransformPosWS(VS_INPUT In)
{
	float3 Right = float3(g_View[0][0],g_View[1][0],g_View[2][0]);
	float3 Up = float3(g_View[0][1],g_View[1][1],g_View[2][1]);
	float3 Dir = float3(g_View[0][2],g_View[1][2],g_View[2][2]);
	float4 Off = float4(RotateAngleAxis(
		Up * lerp(In.Tex2.y * 0.5, -In.Tex2.y * 0.5, In.Tex0.y) + Right * lerp(-In.Tex2.x * 0.5, In.Tex2.x * 0.5, In.Tex0.x),
		In.Tex2.z, Dir), 0);
	return Off + mul(float4(In.Pos.xyz + In.Velocity * (g_Time - In.Tex2.w), In.Pos.w), g_World);
}

float4 TransformPos(VS_INPUT In)
{
	return mul(TransformPosWS(In), g_ViewProj);
}

float4 TransformPosShadow(VS_INPUT In)
{
	return mul(TransformPosWS(In), g_SkyLightViewProj);
}

float2 TransformUV(VS_INPUT In)
{
	return In.Tex0.xy;
}

float3 TransformNormal(VS_INPUT In)
{
	return -float3(g_View[0][2],g_View[1][2],g_View[2][2]);
}

float3 TransformTangent(VS_INPUT In)
{
	return float3(0,0,0);
}

float4 TransformLightWS(VS_INPUT In)
{
	return float4(mul(In.Pos, g_World).xyz, In.Tex2.x * 0.5);
}

float4 TransformColor(VS_INPUT In)
{
	return In.Color;
}
