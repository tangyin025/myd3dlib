
#define WAVE_LENGTH0 3.0
#define WAVE_LENGTH1 3.0
#define WAVE_LENGTH2 3.0
#define PI 3.141596

float FresExp = 3.0;
float ReflStrength = 3.4;
float Amplitude[3] = {0.03, 0.03, 0.03};
float Frequency[3] = {2*PI/WAVE_LENGTH0, 2*PI/WAVE_LENGTH1, 2*PI/WAVE_LENGTH2};
float Phase[3] = {0.2*2*PI/WAVE_LENGTH0, 0.2*2*PI/WAVE_LENGTH1, 0.2*2*PI/WAVE_LENGTH2};
float GerstnerQ[3] = {0.6, 0.6, 0.6};
float3 WaveDir[3] = { {1.0, 0, 1.0}, {1.0, 0, -1.0}, {1.0, 0, 1.0} };

struct TRANSPARENT_VS_OUTPUT
{
	float4 pos			: POSITION;
	float2 texCoord0	: TEXCOORD0;
	float2 texCoord1	: TEXCOORD1;
	float2 texCoord2	: TEXCOORD2;
	float3 NormalWS		: TEXCOORD3;
	float3 TangentWS	: TEXCOORD4;
	float3 BinormalWS	: TEXCOORD5;
	float3 ViewWS		: TEXCOORD6;
};

TRANSPARENT_VS_OUTPUT TransparentVS( VS_INPUT In )
{
	TRANSPARENT_VS_OUTPUT Out;
	float4 PosWS = TransformPosWS(In);
	float2 Tex0 = TransformUV(In);
	float3 NormalWS = mul(TransformNormal(In), (float3x3)g_View);
	float3 TangentWS = mul(TransformTangent(In), (float3x3)g_View);
	float3 BinormalWS = cross(NormalWS, TangentWS);
	
	uint i = 0;
	for(i = 0; i < 3; i++)
	{
		float angle = Frequency[i] * dot(WaveDir[i], PosWS.xyz) + Phase[i] * g_Time;
		float S = sin(angle);
		float C = cos(angle);
		float WA = Frequency[i] * Amplitude[i];

		PosWS.x += WaveDir[i].x * GerstnerQ[i] * Amplitude[i] * C;
		PosWS.y += Amplitude[i] * S;
		PosWS.z += WaveDir[i].z * GerstnerQ[i] * Amplitude[i] * C;
		
		BinormalWS.x -= GerstnerQ[i] * WaveDir[i].x * WaveDir[i].x * WA * S;
		BinormalWS.y += WaveDir[i].z * WA * C;
		BinormalWS.z -= GerstnerQ[i] * WaveDir[i].x * WaveDir[i].z * WA * S;
		
		TangentWS.x -= GerstnerQ[i] * WaveDir[i].x * WaveDir[i].z * WA * S;
		TangentWS.y += WaveDir[i].x * WA * C;
		TangentWS.z -= GerstnerQ[i] * WaveDir[i].z * WaveDir[i].z * WA * S;
		
		NormalWS.x -= WaveDir[i].x * WA * C;
		NormalWS.y -= GerstnerQ[i] * WA * S;
		NormalWS.z -= WaveDir[i].z * WA * C;
	}

	Out.pos = mul(PosWS, g_ViewProj);
	Out.texCoord0 = Tex0 + g_Time * 0.02;
	Out.texCoord1 = Tex0 * 2.0 + g_Time * -0.02;
	Out.texCoord2 = Tex0 / 2.0 + g_Time * 0.01;
	Out.BinormalWS = BinormalWS;
	Out.TangentWS = TangentWS;
	Out.NormalWS = NormalWS;
	Out.ViewWS = g_Eye - PosWS;
	return Out;
}

float4 TransparentPS( TRANSPARENT_VS_OUTPUT In ) : COLOR
{
	float3x3 mT2W = float3x3(normalize(In.BinormalWS), normalize(In.TangentWS), normalize(In.NormalWS));
	
	float3 ViewWS = normalize(In.ViewWS);
	
	float3 texNormal0 = tex2D(NormalTextureSampler, In.texCoord0).xyz;
	float3 texNormal1 = tex2D(NormalTextureSampler, In.texCoord1).xyz;
	float3 texNormal2 = tex2D(NormalTextureSampler, In.texCoord2).xyz;
	float3 NormalTS = normalize(2.0 * (texNormal0 + texNormal1 + texNormal2) - 3.0);
	
	float3 NormalWS = mul(NormalTS, mT2W);
	
	float3 ReflectionWS = Reflection(NormalWS, ViewWS);
	
	float4 reflection = texCUBE(ReflectTextureSampler, ReflectionWS);
	
	float fres = Fresnel(NormalWS, ViewWS, FresExp, ReflStrength);
	
	float3 WaterColor = float3(2/255.0, 10/255.0, 31/255.0);
	
	return float4(reflection.xyz * fres + WaterColor * (1 - fres), 1);
}

technique RenderScene
{
    pass PassTypeShadow
    {          
    }
    pass PassTypeNormal
    {          
    }
    pass PassTypeLight
    {          
    }
    pass PassTypeOpaque
    {          
    }
    pass PassTypeTransparent
    {          
		VertexShader = compile vs_3_0 TransparentVS();
		PixelShader  = compile ps_3_0 TransparentPS();
    }
}
