shared float g_Time;
shared float2 g_ScreenDim;
shared float g_ShadowMapSize;
shared float g_ShadowEpsilon;
shared float4x4 g_World;
shared float3 g_Eye;
shared float4x4 g_View;
shared float4x4 g_ViewProj;
shared float4x4 g_InvViewProj;
shared float4x4 g_SkyLightView;
shared float4x4 g_SkyLightViewProj;
shared float4 g_SkyLightColor;
shared float4 g_AmbientColor;
shared texture g_ShadowRT;
shared texture g_NormalRT;
shared texture g_PositionRT;
shared texture g_LightRT;
shared texture g_OpaqueRT;
shared texture g_DownFilterRT;

sampler ShadowRTSampler = sampler_state
{
	Texture = <g_ShadowRT>;
	MipFilter = NONE;
	MinFilter = POINT;
	MagFilter = POINT;
};

sampler NormalRTSampler = sampler_state
{
	Texture = <g_NormalRT>;
	MipFilter = NONE;
	MinFilter = POINT;
	MagFilter = POINT;
};

sampler PositionRTSampler = sampler_state
{
	Texture = <g_PositionRT>;
	MipFilter = NONE;
	MinFilter = POINT;
	MagFilter = POINT;
};

sampler LightRTSampler = sampler_state
{
	Texture = <g_LightRT>;
	MipFilter = NONE;
	MinFilter = POINT;
	MagFilter = POINT;
};

sampler OpaqueRTSampler = sampler_state
{
	Texture = <g_OpaqueRT>;
	MipFilter = NONE;
	MinFilter = POINT;
	MagFilter = POINT;
};

sampler DownFilterRTSampler = sampler_state
{
	Texture = <g_DownFilterRT>;
	MipFilter = Linear;
	MinFilter = Point;
	MagFilter = Linear;
};

float3 Reflection(float3 Normal, float3 View)
{
	return normalize(2 * dot(Normal, View) * Normal - View);
}

float Fresnel(float3 Normal, float3 View, float FresExp, float ReflStrength)
{
	return pow(1.0 - abs(dot(Normal, View)), FresExp) * ReflStrength;
}

float4 AlignUnit(float4 pos)
{
	return float4(((floor((g_ScreenDim.x + pos.x / pos.w * g_ScreenDim.x) * 0.5 + 0.222222) - 0.5) * 2 - g_ScreenDim.x) / g_ScreenDim.x * pos.w, (g_ScreenDim.y - (floor((g_ScreenDim.y - pos.y / pos.w * g_ScreenDim.y) * 0.5 + 0.222222) - 0.5) * 2) / g_ScreenDim.y * pos.w, pos.z, pos.w);
}

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

float GetLigthAmount(float4 PosShadow)
{
	float2 ShadowTexC = PosShadow.xy / PosShadow.w * 0.5 + 0.5;
	ShadowTexC.y = 1.0 - ShadowTexC.y;
	if (ShadowTexC.x < 0 || ShadowTexC.x > 1 || ShadowTexC.y < 0 || ShadowTexC.y > 1)
		return 1.0;
	
	float LightAmount = 0;
	float x, y;
	for(x = -0.0; x <= 1.0; x += 1.0)
		for(y = -0.0; y <= 1.0; y+= 1.0)
			LightAmount += tex2D(ShadowRTSampler, ShadowTexC + float2(x, y) / g_ShadowMapSize).x + g_ShadowEpsilon < PosShadow.z / PosShadow.w ? 0.0 : 1.0;
			
	return LightAmount / 4;
}

float ScreenDoorTransparency(float Alpha, float2 SPos)
{
	float4x4 thresholdMatrix =
	{
		1.0 / 17.0, 9.0 / 17.0, 3.0 / 17.0, 11.0 / 17.0,
		13.0 / 17.0, 5.0 / 17.0, 15.0 / 17.0, 7.0 / 17.0,
		4.0 / 17.0, 12.0 / 17.0, 2.0 / 17.0, 10.0 / 17.0,
		16.0 / 17.0, 8.0 / 17.0, 14.0 / 17.0, 6.0 / 17.0
	};
	return Alpha - thresholdMatrix[SPos.x % 4][SPos.y % 4];
}
