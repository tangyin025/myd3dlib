
float g_FresExp:MaterialParameter = 4.0;
float g_ReflStrength:MaterialParameter = 1.0;
texture g_NormalTexture:MaterialParameter<string Initialize="texture/water_bump.dds";>;
float3 g_refraction_color:MaterialParameter = {0.00784,0.03921,0.12156};
float3 g_reflection_color:MaterialParameter = {0.8,0.88,0.89};
float3 g_WaterScatterColor:MaterialParameter = {0.3,0.7,0.6};
float g_SpecularExp:MaterialParameter = 100;

sampler NormalTextureSampler = sampler_state
{
	Texture = <g_NormalTexture>;
	MipFilter = LINEAR;
	MinFilter = LINEAR;
	MagFilter = LINEAR;
    ADDRESSU = WRAP;
    ADDRESSV = WRAP;
};

struct TRANSPARENT_VS_OUTPUT
{
	float4 Pos			: POSITION;
	float4 PosWS		: TEXCOORD6;
	float2 texCoord0	: TEXCOORD0;
	float2 texCoord1	: TEXCOORD1;
	float2 texCoord2	: TEXCOORD2;
	float3 Normal		: NORMAL;
	float3 Tangent		: TEXCOORD3;
	float3 Binormal		: TEXCOORD4;
	float3 ViewWS		: TEXCOORD5;
};

TRANSPARENT_VS_OUTPUT TransparentVS( VS_INPUT In )
{
	TRANSPARENT_VS_OUTPUT Out;
	Out.PosWS = TransformPosWS(In);
	float2 Tex0 = TransformUV(In);
	Out.texCoord0 = Tex0 /2.0 + g_Time * 0.02;
	Out.texCoord1 = Tex0 /2.0 + g_Time * -0.02;
	Out.texCoord2 = Tex0 /4.0 + g_Time * 0.01;
	Out.Normal = TransformNormal(In);
	Out.Tangent = TransformTangent(In);
	Out.Binormal = cross(Out.Tangent, Out.Normal); // ! left handed water_bump.dds
	Out.ViewWS = g_Eye - Out.PosWS.xyz; // ! dont normalize here
	
	float3x3 m = float3x3(Out.Tangent,Out.Binormal,Out.Normal);
	float4 nt0 = tex2Dlod(NormalTextureSampler, float4(Out.texCoord0,0,0));
	float4 nt1 = tex2Dlod(NormalTextureSampler, float4(Out.texCoord1,0,0));
	float4 nt2 = tex2Dlod(NormalTextureSampler, float4(Out.texCoord2,0,0));
	float3 nt = normalize(2.0 * (nt0.xyz + nt1.xyz + nt2.xyz) - 3.0);
	float3 n = mul(nt,m);
	Out.PosWS.y += (nt0.w + nt1.w + nt2.w) - 1.5;
	Out.PosWS.xz -= n.xz*0.5;
	Out.Pos = mul(Out.PosWS, g_ViewProj);
	
	return Out;
}

float4 TransparentPS( TRANSPARENT_VS_OUTPUT In ) : COLOR
{
	float3 SkyLightDir = normalize(float3(g_SkyLightView[0][2], g_SkyLightView[1][2], g_SkyLightView[2][2]));
	float3x3 m = float3x3(In.Tangent,In.Binormal,In.Normal);
	float3 nt0 = tex2D(NormalTextureSampler, In.texCoord0).xyz;
	float3 nt1 = tex2D(NormalTextureSampler, In.texCoord1).xyz;
	float3 nt2 = tex2D(NormalTextureSampler, In.texCoord2).xyz;
	float3 nt = normalize(2.0 * (nt0 + nt1 + nt2) - 3.0);
	float3 Normal = mul(nt, m);
	float c = dot(Normal, SkyLightDir);
	float pixel_to_eye_length = length(In.ViewWS);
	float3 pixel_to_eye_vector = In.ViewWS / pixel_to_eye_length;
	float fresnel_factor = Fresnel(Normal, pixel_to_eye_vector, g_FresExp, g_ReflStrength);

	// // calculating fresnel factor 
	// float r=(1.2-1.0)/(1.2+1.0);
	// float fresnel_factor = max(0.0,min(1.0,r+(1.0-r)*pow(1.0-dot(Normal,pixel_to_eye_vector),4)));

	float3 color=lerp(g_refraction_color,g_reflection_color,fresnel_factor);
	float3 Ref = Reflection(Normal, pixel_to_eye_vector);
	float specular = pow(saturate(dot(Ref, SkyLightDir)), g_SpecularExp) * g_SkyLightColor.w;

	// only the crests of water waves generate double refracted light
	float scatter_factor=2.5*max(0,In.PosWS.y*0.25+0.25);

	// the waves that lie between camera and light projection on water plane generate maximal amount of double refracted light 
	scatter_factor*=pow(max(0.0,dot(normalize(float3(SkyLightDir.x,0.0,SkyLightDir.z)),-pixel_to_eye_vector)),2.0);
	
	// the slopes of waves that are oriented back to light generate maximal amount of double refracted light 
	// scatter_factor*=pow(max(0.0,1-dot(SkyLightDir,Normal)),8.0);
	scatter_factor*=pow(1-dot(SkyLightDir,Normal),8);
	
	// water crests gather more light than lobes, so more light is scattered under the crests
	scatter_factor+=1.5*0.2*max(0,In.PosWS.y+1)*
		// the scattered light is best seen if observing direction is normal to slope surface
		max(0,dot(pixel_to_eye_vector,Normal))*
		// fading scattered light out at distance and if viewing direction is vertical to avoid unnatural look
		max(0,1-pixel_to_eye_vector.y)*(300.0/(300+pixel_to_eye_length));
		
	// return float4(g_WaterScatterColor*scatter_factor,1);
	return float4(color+specular+g_WaterScatterColor*scatter_factor,1);
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
