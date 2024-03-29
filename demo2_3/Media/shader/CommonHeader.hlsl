// Registers - vs_3_0
// Register		Name						Count
// v#			Input Register				16
// r#			Temporary Register			32
// c#			Constant Float Register		D3DCAPS9.MaxVertexShaderConst (at least 256)
// a0			Address Register			1
// b#			Constant Boolean Register	16
// i#			Constant Integer Register	16
// aL			Loop Counter Register		1
// p0			Predicate Register			1
// s#			Sampler (Direct3D 9 asm-vs)	4
// o#			Output Register				12

// ps_3_0 Registers
// Register		Name						Count
// v#			Input Register				10
// r#			Temporary Register			32
// c#			Constant Float Register		224
// i#			Constant Integer Register	16
// b#			Constant Boolean Register	16
// p0			Predicate Register			1
// s#			Sampler (Direct3D 9 asm-ps)	16
// vFace		Face_Register				1
// vPos			Position_Register			1
// aL			Loop_Counter_Register		1 
// oC#			Output Color Register		See Multiple-element Textures (Direct3D 9)
// oDepth		Output Depth Register		1

shared float g_Time;
shared float2 g_ScreenDim;
shared float g_ShadowMapSize;
shared float g_ShadowBias;
shared float4x4 g_World;
shared float3 g_Eye;
shared float4x4 g_View;
shared float4x4 g_ViewProj;
shared float4x4 g_SkyLightView;
shared float4x4 g_SkyLightViewProj;
shared float4 g_SkyLightColor;
shared float4 g_AmbientColor;
shared texture g_ShadowRT;
shared texture g_NormalRT;
shared texture g_PositionRT;
shared texture g_SpecularRT;
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

sampler SpecularRTSampler = sampler_state
{
	Texture = <g_SpecularRT>;
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

float GetLigthAmount(float4 ShadowCoord)
{
	//transform from RT space to texture space.
	float2 ShadowTex = ShadowCoord.xy / ShadowCoord.w * 0.5 + 0.5;
	ShadowTex.y = 1.0 - ShadowTex.y;
	if (ShadowTex.x < 0 || ShadowTex.x > 1 || ShadowTex.y < 0 || ShadowTex.y > 1)
		return 1.0;

	// transform to texel space
	float2 texelpos = ShadowTex * g_ShadowMapSize;
	
	// Determine the lerp amounts           
	float2 lerps = frac( texelpos );
	
	//read in bilerp stamp, doing the shadow checks
	float sourcevals[4];
	sourcevals[0] = (tex2Dlod( ShadowRTSampler, float4(ShadowTex,0,0) ).r - g_ShadowBias > ShadowCoord.z / ShadowCoord.w)? 0.0f: 1.0f;  
	sourcevals[1] = (tex2Dlod( ShadowRTSampler, float4(ShadowTex + float2(1.0/g_ShadowMapSize, 0),0,0) ).r - g_ShadowBias > ShadowCoord.z / ShadowCoord.w)? 0.0f: 1.0f;  
	sourcevals[2] = (tex2Dlod( ShadowRTSampler, float4(ShadowTex + float2(0, 1.0/g_ShadowMapSize),0,0) ).r - g_ShadowBias > ShadowCoord.z / ShadowCoord.w)? 0.0f: 1.0f;  
	sourcevals[3] = (tex2Dlod( ShadowRTSampler, float4(ShadowTex + float2(1.0/g_ShadowMapSize, 1.0/g_ShadowMapSize),0,0) ).r - g_ShadowBias > ShadowCoord.z / ShadowCoord.w)? 0.0f: 1.0f;  
        
	// lerp between the shadow values to calculate our light amount
	float LightAmount = lerp( lerp( sourcevals[0], sourcevals[1], lerps.x ),
							  lerp( sourcevals[2], sourcevals[3], lerps.x ),
							  lerps.y );
			
	return LightAmount;
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

float2 TiledUv(float2 uv, int2 tiles, int id)
{
	int frame = id % (tiles.x * tiles.y);
	float2 pos = float2((float)frame % tiles.x, frame / tiles.x); // ! issue occurred if without type cast
	return (pos + uv) / tiles;
}

float SplineInterpolate(float t)
{
	return t + t * (1 - t) * ((t - 1) + t);
}
