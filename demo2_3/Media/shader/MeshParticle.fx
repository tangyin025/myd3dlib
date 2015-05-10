
float3 g_ParticleDir;
float3 g_ParticleUp;
float3 g_ParticleRight;
float2 g_AnimationColumnRow;

struct VS_INPUT
{
	float4 Pos				: POSITION;
	float4 Color			: COLOR0;
	float2 Tex0  			: TEXCOORD0;
	float4 Tex1				: TEXCOORD1;
	float4 Tex2				: TEXCOORD2;
};

float3 RotateAngleAxis(float3 v, float a, float3 N)
{
	float sin_a, cos_a;
	sincos(a, sin_a, cos_a);
	float Nxx = N.x * N.x;
	float Nyy = N.y * N.y;
	float Nzz = N.z * N.z;
	float Nxy = N.x * N.y;
	float Nyz = N.y * N.z;
	float Nzx = N.z * N.x;
	
	float3x3 mRotation = {
		Nxx * (1 - cos_a) + cos_a,			Nxy * (1 - cos_a) + N.z * sin_a,	Nzx * (1 - cos_a) - N.y * sin_a,
		Nxy * (1 - cos_a) - N.z * sin_a,	Nyy * (1 - cos_a) + cos_a,			Nyz * (1 - cos_a) + N.x * sin_a,
		Nzx * (1 - cos_a) + N.y * sin_a,	Nyz * (1 - cos_a) - N.x * sin_a,	Nzz * (1 - cos_a) + cos_a};
		
	return mul(v, mRotation);
}

float4 TransformPosWS(VS_INPUT In)
{
	float4 Off = float4(RotateAngleAxis(
		g_ParticleUp * lerp(In.Tex1.y * 0.5, -In.Tex1.y * 0.5, In.Tex0.y) + g_ParticleRight * lerp(-In.Tex1.x * 0.5, In.Tex1.x * 0.5, In.Tex0.x),
		In.Tex1.z, g_ParticleDir), 0);
	return Off + mul(In.Pos, g_World);
}

float4 TransformPos(VS_INPUT In)
{
	return mul(TransformPosWS(In), g_ViewProj);
}

float2 TransformUV(VS_INPUT In)
{
	return (In.Tex2.xy + In.Tex0.xy) / g_AnimationColumnRow;
}

float3 TransformNormal(VS_INPUT In)
{
	return g_ParticleDir;
}

float3 TransformTangent(VS_INPUT In)
{
	return float3(0);
}

float4 TransformLight(VS_INPUT In)
{
	return float4(mul(In.Pos, g_World).xyz, In.Tex1.x * 0.5);
}

float4 TransformColor(VS_INPUT In)
{
	return In.Color;
}
