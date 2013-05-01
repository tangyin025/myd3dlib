
#include "CommonHeader.fx"

// ------------------------------------------------------------------------------------------
// global variable
// ------------------------------------------------------------------------------------------

#define WAVE_LENGTH0 3.0
#define WAVE_LENGTH1 3.0
#define WAVE_LENGTH2 3.0
#define PI 3.141596

texture g_WaterNormalTexture;
texture g_CubeTexture;

float FresExp = 3.0;
float ReflStrength = 3.4;
float Amplitude[3] = {0.03, 0.03, 0.03};
float Frequency[3] = {2*PI/WAVE_LENGTH0, 2*PI/WAVE_LENGTH1, 2*PI/WAVE_LENGTH2};
float Phase[3] = {0.2*2*PI/WAVE_LENGTH0, 0.2*2*PI/WAVE_LENGTH1, 0.2*2*PI/WAVE_LENGTH2};
float GerstnerQ[3] = {0.6, 0.6, 0.6};
float3 WaveDir[3] = { {1.0, 0, 1.0}, {1.0, 0, -1.0}, {1.0, 0, 1.0} };

//--------------------------------------------------------------------------------------
// Texture samplers
//--------------------------------------------------------------------------------------

sampler WaterNormalTextureSampler =
sampler_state
{
	Texture = <g_WaterNormalTexture>;
	AddressU = WRAP;
	AddressV = WRAP;
	MipFilter = LINEAR;
	MinFilter = LINEAR;
	MagFilter = LINEAR;
};

samplerCUBE CubeTextureSampler =
sampler_state
{
	Texture = <g_CubeTexture>;
	MipFilter = LINEAR;
	MinFilter = LINEAR;
	MagFilter = LINEAR;
};

// ------------------------------------------------------------------------------------------
// vs
// ------------------------------------------------------------------------------------------

struct VS_OUTPUT
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

VS_OUTPUT WaterEffectVS( in float4 iPos			: POSITION,
						 in float3 iNormal		: NORMAL,
						 in float3 iBinormal	: BINORMAL,
						 in float3 iTangent		: TANGENT,
						 in float2 iTexCoord	: TEXCOORD0 )
{
	VS_OUTPUT Out;
	
	float angle[3];
	float S[3];
	float C[3];
	float4 vPosWS = iPos;
	for(uint i = 0; i < 1; i++)
	{
		angle[i] = Frequency[i] * dot(WaveDir[i], iPos.xyz) + Phase[i] * g_Time;
		S[i] = sin(angle[i]);
		C[i] = cos(angle[i]);
		vPosWS.x += WaveDir[i].x * GerstnerQ[i] * Amplitude[i] * C[i];
		vPosWS.y += Amplitude[i] * S[i];
		vPosWS.z += WaveDir[i].z * GerstnerQ[i] * Amplitude[i] * C[i];
	}
	Out.pos = mul(vPosWS, mul(g_World, g_ViewProj));
	
	Out.texCoord0 = iTexCoord + g_Time * 0.02;
	Out.texCoord1 = iTexCoord * 2.0 + g_Time * -0.02;
	Out.texCoord2 = iTexCoord / 2.0 + g_Time * 0.01;
	
	float3 vBinormalWS = float3(1,0,0);
	float3 vTangentWS = float3(0,0,1);
	float3 vNormalWS = float3(0,1,0);
	for(uint i = 0; i < 1; i++)
	{
		float WA = Frequency[i] * Amplitude[i];
		
		vBinormalWS.x -= GerstnerQ[i] * WaveDir[i].x * WaveDir[i].x * WA * S[i];
		vBinormalWS.y += WaveDir[i].z * WA * C[i];
		vBinormalWS.z -= GerstnerQ[i] * WaveDir[i].x * WaveDir[i].z * WA * S[i];
		
		vTangentWS.x -= GerstnerQ[i] * WaveDir[i].x * WaveDir[i].z * WA * S[i];
		vTangentWS.y += WaveDir[i].x * WA * C[i];
		vTangentWS.z -= GerstnerQ[i] * WaveDir[i].z * WaveDir[i].z * WA * S[i];
		
		vNormalWS.x -= WaveDir[i].x * WA * C[i];
		vNormalWS.y -= GerstnerQ[i] * WA * S[i];
		vNormalWS.z -= WaveDir[i].z * WA * C[i];
	}
	Out.BinormalWS = vBinormalWS;
	Out.TangentWS = vTangentWS;
	Out.NormalWS = vNormalWS;
	Out.ViewWS = g_EyePos - vPosWS;
	return Out;
}

// ------------------------------------------------------------------------------------------
// ps
// ------------------------------------------------------------------------------------------

float4 WaterEffectPS( VS_OUTPUT In ) : COLOR
{
	float3x3 mT2W = float3x3(normalize(In.BinormalWS), normalize(In.TangentWS), normalize(In.NormalWS));
	
	float3 vViewWS = normalize(In.ViewWS);
	
	float3 texNormal0 = tex2D(WaterNormalTextureSampler, In.texCoord0).xyz;
	float3 texNormal1 = tex2D(WaterNormalTextureSampler, In.texCoord1).xyz;
	float3 texNormal2 = tex2D(WaterNormalTextureSampler, In.texCoord2).xyz;
	float3 vNormalTS = normalize(2.0 * (texNormal0 + texNormal1 + texNormal2) - 3.0);
	
	float3 vNormalWS = mul(vNormalTS, mT2W);
	
	float3 vReflectionWS = get_reflection(vNormalWS, vViewWS);
	
	float4 cReflection = texCUBE(CubeTextureSampler, vReflectionWS);
	
	float fres = get_fresnel(vNormalWS, vViewWS, FresExp, ReflStrength);
	
	float3 WaterColor = float3(2/255.0, 10/255.0, 31/255.0);
	
	return float4(cReflection.xyz * fres + WaterColor * (1 - fres), 1);
}

// ------------------------------------------------------------------------------------------
// technique
// ------------------------------------------------------------------------------------------

technique WaterEffect
{
	pass P0
	{
		// AlphaBlendEnable = TRUE;
		// SrcBlend = SRCALPHA;
		// DestBlend = INVSRCALPHA;
		// FillMode = WIREFRAME;
		VertexShader = compile vs_2_0 WaterEffectVS();
		PixelShader = compile ps_2_0 WaterEffectPS();
	}
}
