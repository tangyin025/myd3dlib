#define SHADOW_MAP_SIZE 1024
#define SHADOW_EPSILON 0.00110f

struct VS_OUTPUT
{
	float4 Pos				: POSITION;
	float2 Tex0				: TEXCOORD0;
	float3 Normal			: TEXCOORD1;
	float4 PosLS			: TEXCOORD5;
};

float GetLigthAmount(float4 PosLS)
{
	float2 ShadowTexC = PosLS.xy / PosLS.w * 0.5 + 0.5;
	ShadowTexC.y = 1.0 - ShadowTexC.y;
	
	float LightAmount = 0;
	float x, y;
	for(x = -0.0; x <= 1.0; x += 1.0)
		for(y = -0.0; y <= 1.0; y+= 1.0)
			LightAmount += tex2D(ShadowTextureSampler, ShadowTexC + float2(x, y) / SHADOW_MAP_SIZE) + SHADOW_EPSILON < PosLS.z / PosLS.w ? 0.0f : 1.0f;
			
	return LightAmount / 4;
}

VS_OUTPUT RenderSceneVS( VS_INPUT In )
{
    VS_OUTPUT Output;
	Output.Pos = TransformPos(In);
	Output.Tex0 = TransformUV(In);
	Output.Normal = TransformNormal(In);
	Output.PosLS = mul(TransformPosWS(In), g_SkyLightViewProj);
    return Output;    
}

float4 RenderScenePS( VS_OUTPUT In ) : COLOR0
{ 
    return tex2D(MeshTextureSampler, In.Tex0) * -dot(In.Normal, g_SkyLightDir) * GetLigthAmount(In.PosLS);
}
