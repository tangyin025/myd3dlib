#include "Quaternion.hlsl"

struct VS_INPUT
{
	float4 Pos0				: POSITION0;
	float2 Tex0  			: TEXCOORD0; // uv
//#ifdef INSTANCE
	float4 Pos				: POSITION1;
	float4 Velocity			: POSITION2;
	float4 Color			: POSITION3;
	float4 SizeAngleTime	: POSITION4; // size_x, size_y, angle, time
//#endif
};

float4 TransformPosWS(VS_INPUT In)
{
#if EMITTER_FACE_TYPE == 0
	float3 Offset = RotateAngleAxis(
		float3(In.Pos0.z, In.Pos0.y * In.SizeAngleTime.y, -In.Pos0.x * In.SizeAngleTime.x), In.SizeAngleTime.z, float3(1, 0, 0));
#elif EMITTER_FACE_TYPE == 1
	float3 Offset = RotateAngleAxis(
		float3(In.Pos0.x * In.SizeAngleTime.x, In.Pos0.z, -In.Pos0.y * In.SizeAngleTime.y), In.SizeAngleTime.z, float3(0, 1, 0));
#elif EMITTER_FACE_TYPE == 2
	float3 Offset = RotateAngleAxis(
		float3(In.Pos0.x * In.SizeAngleTime.x, In.Pos0.y * In.SizeAngleTime.y, In.Pos0.z), In.SizeAngleTime.z, float3(0, 0, 1));
#elif EMITTER_FACE_TYPE == 3
	float3 Right = float3(g_View[0][0],g_View[1][0],g_View[2][0]);
	float3 Up = float3(g_View[0][1],g_View[1][1],g_View[2][1]);
	float3 Dir = float3(g_View[0][2],g_View[1][2],g_View[2][2]);
	float3 Offset = RotateAngleAxis(
		Right * In.Pos0.x * In.SizeAngleTime.x + Up * In.Pos0.y * In.SizeAngleTime.y + Dir * In.Pos0.z, In.SizeAngleTime.z, Dir);
#elif EMITTER_FACE_TYPE == 4
	float s, c;
	sincos(In.SizeAngleTime.z, s, c);
	float3 Right = float3(c, 0, -s);
	float3 Up = float3(0, 1, 0);
	float3 Dir = float3(s, 0, c);
	float3 Offset =
		Right * In.Pos0.x * In.SizeAngleTime.x + Up * In.Pos0.y * In.SizeAngleTime.y + Dir * In.Pos0.z;
#elif EMITTER_FACE_TYPE == 5
	float3 Up = float3(0, 1, 0);
	float3 Dir = float3(g_View[0][2],g_View[1][2],g_View[2][2]);
	float3 Right = normalize(cross(Up, Dir));
	Dir = cross(Right, Up);
	float3 Offset = RotateAngleAxis(
		Right * In.Pos0.x * In.SizeAngleTime.x + Up * In.Pos0.y * In.SizeAngleTime.y + Dir * In.Pos0.z, In.SizeAngleTime.z, Dir);
#endif
#if EMITTER_VEL_TYPE == 1
	float4 Pos = mul(float4(In.Pos.xyz + In.Velocity.xyz * (g_Time - In.SizeAngleTime.w), In.Pos.w), g_World);
#elif EMITTER_VEL_TYPE == 2
	float4 Pos = mul(In.Pos, g_World);
	Offset = rotate_vector(Offset, In.Velocity);
#else
	float4 Pos = mul(In.Pos, g_World);
#endif
	Pos.xyz += Offset;
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
	return In.Tex0.xy;
}

float3 TransformNormal(VS_INPUT In)
{
#if EMITTER_FACE_TYPE == 0
	float3 Normal = float3(1,0,0);
#elif EMITTER_FACE_TYPE == 1
	float3 Normal = float3(0,1,0);
#elif EMITTER_FACE_TYPE == 2
	float3 Normal = float3(0,0,1);
#elif EMITTER_FACE_TYPE == 3
	float3 Normal = float3(g_View[0][2],g_View[1][2],g_View[2][2]);
#elif EMITTER_FACE_TYPE == 4
	float s, c;
	sincos(In.SizeAngleTime.z, s, c);
	float3 Normal = float3(c, 0, -s);
#elif EMITTER_FACE_TYPE == 5
	float3 Up = float3(0, 1, 0);
	float3 Dir = float3(g_View[0][2],g_View[1][2],g_View[2][2]);
	float3 Right = normalize(cross(Up, Dir));
	float3 Normal = cross(Right, Up);
#endif
#if EMITTER_VEL_TYPE == 2
	Normal = rotate_vector(Normal, In.Velocity);
#endif
return Normal;
}

float3 TransformTangent(VS_INPUT In)
{
#if EMITTER_FACE_TYPE == 0
	float3 Tangent = float3(0,0,-1);
#elif EMITTER_FACE_TYPE == 1
	float3 Tangent = float3(1,0,0);
#elif EMITTER_FACE_TYPE == 2
	float3 Tangent = float3(1,0,0);
#elif EMITTER_FACE_TYPE == 3
	float3 Tangent = float3(g_View[0][0],g_View[1][0],g_View[2][0]);
#elif EMITTER_FACE_TYPE == 4
	float s, c;
	sincos(In.SizeAngleTime.z, s, c);
	float3 Tangent = float3(-s, 0, -c);
#elif EMITTER_FACE_TYPE == 5
	float3 Up = float3(0, 1, 0);
	float3 Dir = float3(g_View[0][2],g_View[1][2],g_View[2][2]);
	float3 Tangent = normalize(cross(Up, Dir));
#endif
#if EMITTER_VEL_TYPE == 2
	Tangent = rotate_vector(Tangent, In.Velocity);
#endif
	return Tangent;
}

float4 TransformLightWS(VS_INPUT In)
{
	return float4(mul(In.Pos, g_World).xyz, In.SizeAngleTime.x * 0.5);
}

float4 TransformColor(VS_INPUT In)
{
	return In.Color;
}