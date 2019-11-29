
struct VS_INPUT
{
	float4 Pos0				: POSITION0;
	float2 Tex0  			: TEXCOORD0; // uv
//#ifdef INSTANCE
	float4 Pos				: POSITION1;
	float3 Velocity			: NORMAL1;
	float4 Color			: POSITION2;
	float4 SizeAngleTime	: POSITION3; // size_x, size_y, angle, time
//#endif
};

float3 g_ParticleOffset;

float4 TransformPosWS(VS_INPUT In)
{
// #ifdef FACETOCAMERA
	// float3 Right = float3(g_View[0][0],g_View[1][0],g_View[2][0]);
	// float3 Up = float3(g_View[0][1],g_View[1][1],g_View[2][1]);
	// float3 Dir = float3(g_View[0][2],g_View[1][2],g_View[2][2]);
	// float4 Offset = float4(RotateAngleAxis(
		// Up * lerp(In.SizeAngleTime.y * 0.5, -In.SizeAngleTime.y * 0.5, In.Tex0.y + g_ParticleOffset.y) + Right * lerp(-In.SizeAngleTime.x * 0.5, In.SizeAngleTime.x * 0.5, In.Tex0.x + g_ParticleOffset.x) + Dir * g_ParticleOffset.z, In.SizeAngleTime.z, Dir), 0);
// #else
	// float s, c;
	// sincos(In.SizeAngleTime.z, s, c);
	// float3 Right = float3(c, 0, s);
	// float3 Up = float3(0, 1, 0);
	// float3 Dir = float3(-s, 0, c);
	// float4 Offset = float4(
		// Up * lerp(In.SizeAngleTime.y * 0.5, -In.SizeAngleTime.y * 0.5, In.Tex0.y + g_ParticleOffset.y) + Right * lerp(-In.SizeAngleTime.x * 0.5, In.SizeAngleTime.x * 0.5, In.Tex0.x + g_ParticleOffset.x) + Dir * g_ParticleOffset.z, 0);
// #endif
	float4 Center = mul(float4(In.Pos.xyz + In.Velocity * (g_Time - In.SizeAngleTime.w), In.Pos.w), g_World);
	// return Center + Offset;
	return Center + In.Pos0;
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
// #ifdef FACETOCAMERA
	// float3 Dir = float3(g_View[0][2],g_View[1][2],g_View[2][2]);
	// return Dir;
// #else
	// float s, c;
	// sincos(In.SizeAngleTime.z, s, c);
	// float3 Dir = float3(-s, 0, c);
	// return Dir;
// #endif
	return float3(0,0,1);
}

float3 TransformTangent(VS_INPUT In)
{
// #ifdef FACETOCAMERA
	// float3 Right = float3(g_View[0][0],g_View[1][0],g_View[2][0]);
	// return Right;
// #else
	// float s, c;
	// sincos(In.SizeAngleTime.z, s, c);
	// float3 Right = float3(c, 0, s);
	// return Right;
// #endif
	return float3(1,0,0);
}

float4 TransformLightWS(VS_INPUT In)
{
	return float4(mul(In.Pos, g_World).xyz, In.SizeAngleTime.x * 0.5);
}

float4 TransformColor(VS_INPUT In)
{
	return In.Color;
}
