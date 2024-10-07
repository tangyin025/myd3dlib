#include <CommonHeader.hlsl>

#ifdef TILED
int2 g_Tiles;
#endif

struct VS_INPUT
{
	float4 Pos				: POSITION0;
	float2 Tex0  			: TEXCOORD0; // uv
	float3 Normal			: NORMAL;
	float3 Tangent			: TANGENT;
//#ifdef INSTANCE
	float4 PosInst			: POSITION1;
	float4 Velocity			: POSITION2;
	float4 Color			: POSITION3;
	float4 SizeAngleTime	: POSITION4; // size_x, size_y, angle, time
//#endif
};

float4 TransformPosWS(VS_INPUT In)
{
#if EMITTER_FACE_TYPE == 0 // FaceTypeX
    float3 Offset = RotateAngleAxis(
        float3(In.Pos.z * In.SizeAngleTime.x, In.Pos.y * In.SizeAngleTime.y, -In.Pos.x * In.SizeAngleTime.x), In.SizeAngleTime.z, float3(1, 0, 0));
#elif EMITTER_FACE_TYPE == 1 // FaceTypeY
    float3 Offset = RotateAngleAxis(
        float3(In.Pos.x * In.SizeAngleTime.x, In.Pos.z * In.SizeAngleTime.x, -In.Pos.y * In.SizeAngleTime.y), In.SizeAngleTime.z, float3(0, 1, 0));
#elif EMITTER_FACE_TYPE == 2 // FaceTypeZ
    float3 Offset = RotateAngleAxis(
        float3(In.Pos.x * In.SizeAngleTime.x, In.Pos.y * In.SizeAngleTime.y, In.Pos.z * In.SizeAngleTime.x), In.SizeAngleTime.z, float3(0, 0, 1));
#elif EMITTER_FACE_TYPE == 3 // FaceTypeCamera
	// 注意，右手系空间Dir朝着屏幕向外
	float3 Right = float3(g_View[0][0],g_View[1][0],g_View[2][0]);
	float3 Up = float3(g_View[0][1],g_View[1][1],g_View[2][1]);
	float3 Dir = float3(g_View[0][2],g_View[1][2],g_View[2][2]);
    float3 Offset = RotateAngleAxis(
        Right * In.Pos.x * In.SizeAngleTime.x + Up * In.Pos.y * In.SizeAngleTime.y + Dir * In.Pos.z * In.SizeAngleTime.x, In.SizeAngleTime.z, Dir);
#elif EMITTER_FACE_TYPE == 4 // FaceTypeAngle
	float s, c;
	sincos(In.SizeAngleTime.z, s, c);
	float3 Right = float3(c, 0, -s);
	float3 Up = float3(0, 1, 0);
	float3 Dir = float3(s, 0, c);
    float3 Offset =
        Right * In.Pos.x * In.SizeAngleTime.x + Up * In.Pos.y * In.SizeAngleTime.y + Dir * In.Pos.z * In.SizeAngleTime.x;
#elif EMITTER_FACE_TYPE == 5 // FaceTypeAngleCamera
	float3 Right = float3(g_View[0][0],g_View[1][0],g_View[2][0]);
	float3 Up = float3(0, 1, 0);
	float3 Dir = normalize(cross(Right, Up));
    float3 Offset = RotateAngleAxis(
        Right * In.Pos.x * In.SizeAngleTime.x + Up * In.Pos.y * In.SizeAngleTime.y + Dir * In.Pos.z * In.SizeAngleTime.x, In.SizeAngleTime.z, Dir);
#elif EMITTER_FACE_TYPE == 6 // FaceTypeStretchedCamera
    float3 Up = normalize(In.Velocity.xyz);
    float3 Right = normalize(cross(Up, float3(g_View[0][2], g_View[1][2], g_View[2][2])));
    float3 Dir = cross(Right, Up);
    float3 Offset = RotateAngleAxis(
        Right * In.Pos.x * In.SizeAngleTime.x + Up * In.Pos.y * In.SizeAngleTime.y + Dir * In.Pos.z * In.SizeAngleTime.x, In.SizeAngleTime.z, Dir);
#endif
	float4 Pos = mul(In.PosInst, g_World) + float4(Offset, 0);
	return Pos;
}

float4 TransformPos(VS_INPUT In)
{
	return mul(TransformPosWS(In), g_ViewProj);
}

float2 TransformUV(VS_INPUT In)
{
#ifdef TILED
    int frame = In.Color.r * 255 % (g_Tiles.x * g_Tiles.y);
    float2 pos = float2((float)frame % g_Tiles.x, frame / g_Tiles.x); // ! issue occurred if without type cast
    return (pos + In.Tex0.xy) / g_Tiles;
#else
    return In.Tex0.xy;
#endif
}

float3 TransformNormal(VS_INPUT In)
{
#if EMITTER_FACE_TYPE == 0
	float3 Normal = float3(In.Normal.z, In.Normal.y, -In.Normal.x);
#elif EMITTER_FACE_TYPE == 1
	float3 Normal = float3(In.Normal.x, In.Normal.z, -In.Normal.y);
#elif EMITTER_FACE_TYPE == 2
	float3 Normal = In.Normal;
#elif EMITTER_FACE_TYPE == 3
	float3 Right = float3(g_View[0][0],g_View[1][0],g_View[2][0]);
	float3 Up = float3(g_View[0][1],g_View[1][1],g_View[2][1]);
	float3 Dir = float3(g_View[0][2],g_View[1][2],g_View[2][2]);
	float3 Normal = Right * In.Normal.x + Up * In.Normal.y + Dir * In.Normal.z;
#elif EMITTER_FACE_TYPE == 4
	float s, c;
	sincos(In.SizeAngleTime.z, s, c);
	float3 Right = float3(c, 0, -s);
	float3 Up = float3(0, 1, 0);
	float3 Dir = float3(s, 0, c);
	float3 Normal = Right * In.Normal.x + Up * In.Normal.y + Dir * In.Normal.z;
#elif EMITTER_FACE_TYPE == 5
	float3 Right = float3(g_View[0][0],g_View[1][0],g_View[2][0]);
    float3 Up = float3(0, 1, 0);
    float3 Dir = normalize(cross(Right, Up));
    float3 Normal = Right * In.Normal.x + Up * In.Normal.y + Dir * In.Normal.z;
#elif EMITTER_FACE_TYPE == 6 // FaceTypeStretchedCamera
    float3 Up = normalize(In.Velocity.xyz);
    float3 Right = normalize(cross(Up, float3(g_View[0][2], g_View[1][2], g_View[2][2])));
    float3 Dir = cross(Right, Up);
    float3 Normal = Right * In.Normal.x + Up * In.Normal.y + Dir * In.Normal.z;
#endif
	return Normal;
}

float3 TransformTangent(VS_INPUT In)
{
#if EMITTER_FACE_TYPE == 0
	float3 Tangent = float3(In.Tangent.z, In.Tangent.y, -In.Tangent.x);
#elif EMITTER_FACE_TYPE == 1
	float3 Tangent = float3(In.Tangent.x, In.Tangent.z, -In.Tangent.y);
#elif EMITTER_FACE_TYPE == 2
	float3 Tangent = In.Tangent;
#elif EMITTER_FACE_TYPE == 3
	float3 Right = float3(g_View[0][0],g_View[1][0],g_View[2][0]);
	float3 Up = float3(g_View[0][1],g_View[1][1],g_View[2][1]);
	float3 Dir = float3(g_View[0][2],g_View[1][2],g_View[2][2]);
	float3 Tangent = Right * In.Tangent.x + Up * In.Tangent.y + Dir * In.Tangent.z;
#elif EMITTER_FACE_TYPE == 4
	float s, c;
	sincos(In.SizeAngleTime.z, s, c);
	float3 Right = float3(c, 0, -s);
	float3 Up = float3(0, 1, 0);
	float3 Dir = float3(s, 0, c);
	float3 Tangent = Right * In.Tangent.x + Up * In.Tangent.y + Dir * In.Tangent.z;
#elif EMITTER_FACE_TYPE == 5
	float3 Right = float3(g_View[0][0],g_View[1][0],g_View[2][0]);
    float3 Up = float3(0, 1, 0);
    float3 Dir = normalize(cross(Right, Up));
    float3 Tangent = Right * In.Tangent.x + Up * In.Tangent.y + Dir * In.Tangent.z;
#elif EMITTER_FACE_TYPE == 6 // FaceTypeStretchedCamera
    float3 Up = normalize(In.Velocity.xyz);
    float3 Right = normalize(cross(Up, float3(g_View[0][2], g_View[1][2], g_View[2][2])));
    float3 Dir = cross(Right, Up);
    float3 Tangent = Right * In.Tangent.x + Up * In.Tangent.y + Dir * In.Tangent.z;
#endif
    return Tangent;
}

float4 TransformLightWS(VS_INPUT In)
{
	return float4(mul(In.PosInst, g_World).xyz, In.SizeAngleTime.x * 0.5);
}

float4 TransformColor(VS_INPUT In)
{
#ifdef TILED
    return float4(1, 1, 1, 1);
#else
    return In.Color;
#endif
}
