
struct VS_OUTPUT
{
	float4 Pos				: POSITION;
	float4 Pos2				: TEXCOORD0;
	float4 Light			: TEXCOORD1;
};

VS_OUTPUT RenderSceneVS( VS_INPUT In )
{
	VS_OUTPUT Output;
	Output.Pos = TransformPos(In);
	Output.Pos2 = TransformPos(In);
	Output.Light = float4(mul(In.Pos, g_World).xyz, In.Tex1.x * 0.5);
	return Output;
}

float4 RenderScenePS( VS_OUTPUT In ) : COLOR0
{
	float2 xy = In.Pos2.xy / In.Pos2.w * 0.5 + 0.5;
	xy.y = 1 - xy.y;
	float4 Normal = tex2D(NormalTextureSampler, xy);
	float z = Normal.w;
	float4 PosWS = mul(float4(In.Pos2.x, In.Pos2.y, z * In.Pos2.w, In.Pos2.w), g_InvViewProj);
	PosWS.xyz = PosWS.xyz / PosWS.www;
	PosWS.w = 1;
	float3 LightVec = In.Light.xyz - PosWS.xyz;
	float LightLen = length(LightVec);
	LightVec = LightVec / LightLen;
	float3 diffuse = saturate(dot(Normal.xyz, LightVec)) * saturate(1 - LightLen / In.Light.w);
	return float4(diffuse, 0);
}
