
#define WAVE_LENGTH0 3.0
#define WAVE_LENGTH1 3.0
#define WAVE_LENGTH2 3.0
#define PI 3.141596

float g_Amplitude[3] = {0.03, 0.03, 0.03};
float g_Frequency[3] = {2*PI/WAVE_LENGTH0, 2*PI/WAVE_LENGTH1, 2*PI/WAVE_LENGTH2};
float g_Phase[3] = {0.2*2*PI/WAVE_LENGTH0, 0.2*2*PI/WAVE_LENGTH1, 0.2*2*PI/WAVE_LENGTH2};
float g_GerstnerQ[3] = {0.6, 0.6, 0.6};
float3 g_WaveDir[3] = { {1.0, 0, 1.0}, {1.0, 0, -1.0}, {1.0, 0, 1.0} };

float g_FresExp:MaterialParameter = 4.0;
float g_ReflStrength:MaterialParameter = 1.0;
float3 g_WaterColor:MaterialParameter = { 0.00784, 0.03921, 0.12156 };
texture g_NormalTexture:MaterialParameter<string Initialize="texture/water_bump.dds";>;
texture g_ReflectTexture:MaterialParameter<string Initialize="texture/galileo_cross.dds";>;
float g_SpecularExp:MaterialParameter = 25;

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
	float4 PosWS		: POSITION1;
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
	Out.PosWS = TransformPosWS(In);
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

	Out.texCoord0 = Tex0 + g_Time * 0.02;
	Out.texCoord1 = Tex0 * 2.0 + g_Time * -0.02;
	Out.texCoord2 = Tex0 / 2.0 + g_Time * 0.01;
	Out.BinormalWS = BinormalWS;
	Out.TangentWS = TangentWS;
	Out.NormalWS = NormalWS;
	Out.ViewWS = g_Eye - Out.PosWS.xyz; // ! dont normalize here
	
	float4 texNormal0 = tex2Dlod(NormalTextureSampler, float4(Out.texCoord0,0,0));
	float4 texNormal1 = tex2Dlod(NormalTextureSampler, float4(Out.texCoord1,0,0));
	float4 texNormal2 = tex2Dlod(NormalTextureSampler, float4(Out.texCoord2,0,0));
	float3 NormalTS = normalize(2.0 * (texNormal0.xyz + texNormal1.xyz + texNormal2.xyz) - 3.0);
	float3x3 m = float3x3(TangentWS,-BinormalWS,NormalWS);
	float3 n = mul(NormalTS,m);
	Out.PosWS.y+= (texNormal0.w + texNormal1.w + texNormal2.w)*1;
	Out.PosWS.xz-=n.xz*0.5;
	Out.pos = mul(Out.PosWS, g_ViewProj);
	
	return Out;
}

float4 TransparentPS( TRANSPARENT_VS_OUTPUT In ) : COLOR
{
	// float3x3 mT2W = float3x3(In.TangentWS, In.BinormalWS, In.NormalWS);
	// float3 texNormal0 = tex2D(NormalTextureSampler, In.texCoord0).xyz;
	// float3 texNormal1 = tex2D(NormalTextureSampler, In.texCoord1).xyz;
	// float3 texNormal2 = tex2D(NormalTextureSampler, In.texCoord2).xyz;
	// float3 NormalTS = normalize(2.0 * (texNormal0 + texNormal1 + texNormal2) - 3.0);
	// float3 NormalWS = mul(NormalTS, mT2W);
	// float3 ReflectionWS = Reflection(NormalWS, In.ViewWS);
	// float4 reflection = texCUBE(ReflectTextureSampler, ReflectionWS);
	// float fres = Fresnel(NormalWS, In.ViewWS, g_FresExp, g_ReflStrength);
	// return float4(reflection.xyz * fres + g_WaterColor * (1 - fres), 1);
	
	float3 SkyLightDir = normalize(float3(g_SkyLightView[0][2], g_SkyLightView[1][2], g_SkyLightView[2][2]));
	float3x3 mT2W = float3x3(In.TangentWS, -In.BinormalWS, In.NormalWS);
	float3 texNormal0 = tex2D(NormalTextureSampler, In.texCoord0).xyz;
	float3 texNormal1 = tex2D(NormalTextureSampler, In.texCoord1).xyz;
	float3 texNormal2 = tex2D(NormalTextureSampler, In.texCoord2).xyz;
	float3 NormalTS = normalize(2.0 * (texNormal0 + texNormal1 + texNormal2) - 3.0);
	float3 Normal = mul(NormalTS, mT2W);
	float c = dot(Normal, SkyLightDir);
	float pixel_to_eye_length = length(In.ViewWS);
	float3 pixel_to_eye_vector = In.ViewWS / pixel_to_eye_length;
	float fresnel_factor = Fresnel(Normal, pixel_to_eye_vector, g_FresExp, g_ReflStrength);

	// // calculating fresnel factor 
	// float r=(1.2-1.0)/(1.2+1.0);
	// float fresnel_factor = max(0.0,min(1.0,r+(1.0-r)*pow(1.0-dot(Normal,pixel_to_eye_vector),4)));

	float3 reflection_color=float3(0.00784,0.03921,0.12156);
	float3 refraction_color=float3(0.8,0.88,0.89);
	float3 color=lerp(reflection_color,refraction_color,fresnel_factor);
	float3 Ref = Reflection(Normal, pixel_to_eye_vector);
	float specular = pow(saturate(dot(Ref, SkyLightDir)), g_SpecularExp) * g_SkyLightColor.w;

	// float scatter_factor=2.5*max(0,In.PosWS.y*0.25+0.25);

	// // the waves that lie between camera and light projection on water plane generate maximal amount of double refracted light 
	// scatter_factor*=pow(max(0.0,dot(normalize(float3(-SkyLightDir.x,0.0,-SkyLightDir.z)),-pixel_to_eye_vector)),2.0);
	
	// // the slopes of waves that are oriented back to light generate maximal amount of double refracted light 
	// scatter_factor*=pow(max(0.0,1.0-dot(-SkyLightDir,Normal)),8.0);
	
	// water crests gather more light than lobes, so more light is scattered under the crests
	float scatter_factor=1.5*0.2*max(0,In.PosWS.y+1)*
		// the scattered light is best seen if observing direction is normal to slope surface
		max(0,dot(pixel_to_eye_vector,Normal))*
		// fading scattered light out at distance and if viewing direction is vertical to avoid unnatural look
		max(0,1-pixel_to_eye_vector.y)*(300.0/(300+pixel_to_eye_length));

	return float4(color+specular+float3(0.3,0.7,0.6)*scatter_factor,1);
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
