#define SHADOW_MAP_SIZE 1024
#define SHADOW_EPSILON 0.00110f

struct VS_OUTPUT
{
	float4 Pos				: POSITION;
	float2 Tex0				: TEXCOORD0;
	float3 Normal			: TEXCOORD1;
	float4 Pos2				: TEXCOORD2;
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
	Output.Pos2 = Output.Pos;
	Output.PosLS = mul(TransformPosWS(In), g_SkyLightViewProj);
    return Output;    
}

float4 RenderScenePS( VS_OUTPUT In ) : COLOR0
{ 
	float2 DiffuseTex = In.Pos2.xy / In.Pos2.w * 0.5 + 0.5;
	DiffuseTex.y = 1 - DiffuseTex.y;
	float4 Diffuse = tex2D(DiffuseTextureSampler, DiffuseTex);
    return tex2D(MeshTextureSampler, In.Tex0) * saturate(saturate(-dot(In.Normal, g_SkyLightDir) * GetLigthAmount(In.PosLS)) + float4(Diffuse.xyz, 1));
}
