
texture g_NormalTexture;
texture g_ReflectTexture;

sampler NormalTextureSampler = sampler_state
{
	Texture = <g_NormalTexture>;
	MipFilter = LINEAR;
	MinFilter = LINEAR;
	MagFilter = LINEAR;
};

sampler ReflectTextureSampler = sampler_state
{
	Texture = <g_ReflectTexture>;
	MipFilter = LINEAR;
	MinFilter = LINEAR;
	MagFilter = LINEAR;
};

#define WAVE_LENGTH0 3.0
#define WAVE_LENGTH1 3.0
#define WAVE_LENGTH2 3.0
#define PI 3.141596

float g_Amplitude[3] = {0.03, 0.03, 0.03};
float g_Frequency[3] = {2*PI/WAVE_LENGTH0, 2*PI/WAVE_LENGTH1, 2*PI/WAVE_LENGTH2};
float g_Phase[3] = {0.2*2*PI/WAVE_LENGTH0, 0.2*2*PI/WAVE_LENGTH1, 0.2*2*PI/WAVE_LENGTH2};
float g_GerstnerQ[3] = {0.6, 0.6, 0.6};
float3 g_WaveDir[3] = { {1.0, 0, 1.0}, {1.0, 0, -1.0}, {1.0, 0, 1.0} };
float g_FresExp = 3.0;
float g_ReflStrength = 3.4;
float3 g_WaterColor = { 2/255.0, 10/255.0, 31/255.0 };

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
	float3 NormalWS = TransformNormal(In);
	float3 TangentWS = TransformTangent(In);
	float3 BinormalWS = cross(NormalWS, TangentWS);
	
	// uint i = 0;
	// for(i = 0; i < 3; i++)
	// {
		// float angle = g_Frequency[i] * dot(g_WaveDir[i], PosWS.xyz) + g_Phase[i] * g_Time;
		// float S = sin(angle);
		// float C = cos(angle);
		// float WA = g_Frequency[i] * g_Amplitude[i];

		// PosWS.x += g_WaveDir[i].x * g_GerstnerQ[i] * g_Amplitude[i] * C;
		// PosWS.y += g_Amplitude[i] * S;
		// PosWS.z += g_WaveDir[i].z * g_GerstnerQ[i] * g_Amplitude[i] * C;
		
		// BinormalWS.x -= g_GerstnerQ[i] * g_WaveDir[i].x * g_WaveDir[i].x * WA * S;
		// BinormalWS.y += g_WaveDir[i].z * WA * C;
		// BinormalWS.z -= g_GerstnerQ[i] * g_WaveDir[i].x * g_WaveDir[i].z * WA * S;
		
		// TangentWS.x -= g_GerstnerQ[i] * g_WaveDir[i].x * g_WaveDir[i].z * WA * S;
		// TangentWS.y += g_WaveDir[i].x * WA * C;
		// TangentWS.z -= g_GerstnerQ[i] * g_WaveDir[i].z * g_WaveDir[i].z * WA * S;
		
		// NormalWS.x -= g_WaveDir[i].x * WA * C;
		// NormalWS.y -= g_GerstnerQ[i] * WA * S;
		// NormalWS.z -= g_WaveDir[i].z * WA * C;
	// }

	Out.pos = mul(PosWS, g_ViewProj);
	Out.texCoord0 = Tex0 + g_Time * 0.02;
	Out.texCoord1 = Tex0 * 2.0 + g_Time * -0.02;
	Out.texCoord2 = Tex0 / 2.0 + g_Time * 0.01;
	Out.BinormalWS = BinormalWS;
	Out.TangentWS = TangentWS;
	Out.NormalWS = NormalWS;
	Out.ViewWS = normalize(g_Eye - PosWS.xyz);
	return Out;
}

float4 TransparentPS( TRANSPARENT_VS_OUTPUT In ) : COLOR
{
	float3x3 mT2W = float3x3(In.TangentWS, In.BinormalWS, In.NormalWS);
	float3 texNormal0 = tex2D(NormalTextureSampler, In.texCoord0).xyz;
	float3 texNormal1 = tex2D(NormalTextureSampler, In.texCoord1).xyz;
	float3 texNormal2 = tex2D(NormalTextureSampler, In.texCoord2).xyz;
	float3 NormalTS = normalize(2.0 * (texNormal0 + texNormal1 + texNormal2) - 3.0);
	float3 NormalWS = mul(NormalTS, mT2W);
	float3 ReflectionWS = Reflection(NormalWS, In.ViewWS);
	float4 reflection = texCUBE(ReflectTextureSampler, ReflectionWS);
	float fres = Fresnel(NormalWS, In.ViewWS, g_FresExp, g_ReflStrength);
	return float4(reflection.xyz * fres + g_WaterColor * (1 - fres), 1);
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
