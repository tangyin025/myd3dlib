
struct VS_OUTPUT
{
	float4 Pos				: POSITION;
	float2 Tex0				: TEXCOORD0;
	float3 Normal			: TEXCOORD1;
#ifdef TEXTURE_TYPE_NORMAL
	float3 Tangent			: TEXCOORD2;
	float3 Binormal			: TEXCOORD3;
#endif
};

VS_OUTPUT RenderSceneVS( VS_INPUT In )
{
	VS_OUTPUT Output;
	Output.Pos = TransformPos(In);
	Output.Tex0 = Output.Pos.zw;
	Output.Normal = TransformNormal(In);
#ifdef TEXTURE_TYPE_NORMAL
	Output.Tangent = TransformTangent(In);
	Output.Binormal = cross(Output.Normal, Output.Tangent);
#endif
	return Output;
}

float4 RenderScenePS( VS_OUTPUT In ) : COLOR0
{
	return float4(In.Normal.x, In.Normal.y, In.Normal.z, In.Tex0.x / In.Tex0.y);
}
