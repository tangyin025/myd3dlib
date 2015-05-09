
struct VS_OUTPUT
{
	float4 Pos				: POSITION;
	float2 Tex0				: TEXCOORD0;
	float4 Color			: COLOR0;
};

VS_OUTPUT RenderSceneVS( VS_INPUT In )
{
    VS_OUTPUT Output;
	Output.Pos = TransformPos(In);
	Output.Tex0 = TransformUV(In);
	Output.Color = TransformColor(In);
    return Output;    
}

float4 RenderScenePS( VS_OUTPUT In ) : COLOR0
{
	return tex2D(MeshTextureSampler, In.Tex0) * In.Color;
}
