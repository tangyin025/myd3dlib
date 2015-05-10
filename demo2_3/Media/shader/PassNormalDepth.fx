
struct VS_OUTPUT
{
	float4 Pos				: POSITION;
	float2 Tex0				: TEXCOORD0;
	float3 Normal			: TEXCOORD1;
#ifdef TEXTURE_TYPE_NORMAL
	float3 Tangent			: TEXCOORD2;
	float3 Binormal			: TEXCOORD3;
#endif
	float2 Depth			: TEXCOORD4;
};

VS_OUTPUT RenderSceneVS( VS_INPUT In )
{
	VS_OUTPUT Output;
	Output.Pos = TransformPos(In);
	Output.Tex0 = TransformUV(In);
	Output.Normal = TransformNormal(In);
#ifdef TEXTURE_TYPE_NORMAL
	Output.Tangent = TransformTangent(In);
	Output.Binormal = cross(Output.Normal, Output.Tangent);
#endif
	Output.Depth = Output.Pos.zw;
	return Output;
}

float4 RenderScenePS( VS_OUTPUT In ) : COLOR0
{
#ifdef TEXTURE_TYPE_NORMAL
	float3x3 t2w = float3x3(In.Tangent, In.Binormal, In.Normal);
	float3 NormalTS = tex2D(NormalTextureSampler, In.Tex0) * 2 - 1;
	float3 Normal = mul(NormalTS, t2w);
#else
	float3 Normal = In.Normal;
#endif
	return float4(Normal.x, Normal.y, Normal.z, In.Depth.x / In.Depth.y);
}
