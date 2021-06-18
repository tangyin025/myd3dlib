
// #define WAVE_LENGTH0 3.0
// #define WAVE_LENGTH1 3.0
// #define WAVE_LENGTH2 3.0
// #define PI 3.141596

// float g_Amplitude[3] = {0.03, 0.03, 0.03};
// float g_Frequency[3] = {2*PI/WAVE_LENGTH0, 2*PI/WAVE_LENGTH1, 2*PI/WAVE_LENGTH2};
// float g_Phase[3] = {0.2*2*PI/WAVE_LENGTH0, 0.2*2*PI/WAVE_LENGTH1, 0.2*2*PI/WAVE_LENGTH2};
// float g_GerstnerQ[3] = {0.6, 0.6, 0.6};
// float3 g_WaveDir[3] = { {1.0, 0, 1.0}, {1.0, 0, -1.0}, {1.0, 0, 1.0} };

float g_FresExp:MaterialParameter = 3.0;
float g_ReflStrength:MaterialParameter = 3.4;
float g_WaterHeight:MaterialParameter = 0.0;
float3 g_WaterColor:MaterialParameter = { 0.00784, 0.03921, 0.12156 };
texture g_NormalTexture:MaterialParameter<string Initialize="texture/WaterNormal2.png";>;
texture g_ReflectTexture:MaterialParameter<string Initialize="texture/galileo_cross.dds";>;

sampler NormalTextureSampler = sampler_state
{
	Texture = <g_NormalTexture>;
	MipFilter = LINEAR;
	MinFilter = LINEAR;
	MagFilter = LINEAR;
    ADDRESSU = WRAP;
    ADDRESSV = WRAP;
};

sampler ReflectTextureSampler = sampler_state
{
	Texture = <g_ReflectTexture>;
	MipFilter = LINEAR;
	MinFilter = LINEAR;
	MagFilter = LINEAR;
    ADDRESSU = WRAP;
    ADDRESSV = WRAP;
};

struct TRANSPARENT_VS_OUTPUT
{
	float4 pos			: POSITION;
	float2 texCoord0	: TEXCOORD0;
	float2 texCoord1	: TEXCOORD1;
	float2 texCoord2	: TEXCOORD2;
	float3 Normal		: TEXCOORD3;
	float3 Tangent		: TEXCOORD4;
	float3 Binormal		: TEXCOORD5;
	float3 ViewWS		: TEXCOORD6;
};

TRANSPARENT_VS_OUTPUT TransparentVS( VS_INPUT In )
{
	TRANSPARENT_VS_OUTPUT Out;
	float4 PosWS = TransformPosWS(In);
	PosWS.y = g_World[3].y + g_WaterHeight;
	float2 Tex0 = TransformUV(In);
	// float3 Normal = TransformNormal(In);
	// float3 Tangent = TransformTangent(In);
	// float3 Binormal = cross(Normal, Tangent);
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
		
		// Binormal.x -= g_GerstnerQ[i] * g_WaveDir[i].x * g_WaveDir[i].x * WA * S;
		// Binormal.y += g_WaveDir[i].z * WA * C;
		// Binormal.z -= g_GerstnerQ[i] * g_WaveDir[i].x * g_WaveDir[i].z * WA * S;
		
		// Tangent.x -= g_GerstnerQ[i] * g_WaveDir[i].x * g_WaveDir[i].z * WA * S;
		// Tangent.y += g_WaveDir[i].x * WA * C;
		// Tangent.z -= g_GerstnerQ[i] * g_WaveDir[i].z * g_WaveDir[i].z * WA * S;
		
		// Normal.x -= g_WaveDir[i].x * WA * C;
		// Normal.y -= g_GerstnerQ[i] * WA * S;
		// Normal.z -= g_WaveDir[i].z * WA * C;
	// }

	Out.pos = mul(PosWS, g_ViewProj);
	Out.texCoord0 = Tex0 + g_Time * 0.02;
	Out.texCoord1 = Tex0 * 2.0 + g_Time * -0.02;
	Out.texCoord2 = Tex0 / 2.0 + g_Time * 0.01;
	Out.Normal = float3(0,1,0);//TransformNormal(In);
	Out.Tangent = float3(1,0,0);//TransformTangent(In);
	Out.Binormal = cross(Out.Normal, Out.Tangent);
	Out.ViewWS = g_Eye - PosWS.xyz; // ! dont normalize here
	return Out;
}

float4 TransparentPS( TRANSPARENT_VS_OUTPUT In ) : COLOR
{
	float3x3 m = float3x3(In.Tangent,-In.Binormal,In.Normal);
	float3 nt0 = tex2D(NormalTextureSampler, In.texCoord0).xyz;
	float3 nt1 = tex2D(NormalTextureSampler, In.texCoord1).xyz;
	float3 nt2 = tex2D(NormalTextureSampler, In.texCoord2).xyz;
	float3 nt = normalize(2.0 * (nt0 + nt1 + nt2) - 3.0);
	float3 Normal = mul(nt, m);
	float3 pixel_to_eye_vector=normalize(In.ViewWS);
	float3 reflection = Reflection(Normal, pixel_to_eye_vector);
	float3 reflection_color = texCUBE(ReflectTextureSampler, reflection).xyz;
	float fres = Fresnel(Normal, pixel_to_eye_vector, g_FresExp, g_ReflStrength);
	return float4(reflection_color * fres + g_WaterColor * (1 - fres), 1);
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
	pass PassTypeBackground
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
