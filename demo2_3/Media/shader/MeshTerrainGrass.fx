
struct VS_INPUT
{
	float4 Pos				: POSITION0;
//#ifdef INSTANCE
	float4 Pos1				: POSITION1;
//#endif
};

float4 TransformPosWS(VS_INPUT In)
{
	float3 Up = float3(0, 1, 0);
	float3 Dir = float3(g_View[0][2],g_View[1][2],g_View[2][2]);
	float3 Right = normalize(cross(Up, Dir));
	Dir = cross(Right, Up);
	float4 Pos = mul(In.Pos1, g_World);
	Pos.xyz += Right * In.Pos.x + Up * In.Pos.y + Dir * In.Pos.z;
	return Pos;
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
	return float2(In.Pos.x, 1 - In.Pos.y);
}

float3 TransformNormal(VS_INPUT In)
{
	float3 Up = float3(0, 1, 0);
	float3 Dir = float3(g_View[0][2],g_View[1][2],g_View[2][2]);
	float3 Right = normalize(cross(Up, Dir));
	Dir = cross(Right, Up);
	return Dir;
}

float3 TransformTangent(VS_INPUT In)
{
	float3 Up = float3(0, 1, 0);
	float3 Dir = float3(g_View[0][2],g_View[1][2],g_View[2][2]);
	float3 Right = normalize(cross(Up, Dir));
	return Right;
}

float4 TransformLightWS(VS_INPUT In)
{
	return float4(In.Pos1.xyz, 1);
}

float4 TransformColor(VS_INPUT In)
{
	return float4(1, 1, 1, 1);
}
