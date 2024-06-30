#include "CommonVert.hlsl"

texture _BaseColorTexture:MaterialParameter<string path="texture/InstancedIndirectGrassVertexColor.jpg";>;
float g_Shininess:MaterialParameter = 25;
float _GrassWidth:MaterialParameter = 1;
float _GrassHeight:MaterialParameter = 1;
float _WindAIntensity:MaterialParameter = 1.77;
float _WindAFrequency:MaterialParameter = 4;
float3 _WindATiling:MaterialParameter = {0.1,0.1,0};
float3 _WindAWrap:MaterialParameter = {0.5,0.5,0};
float _WindBIntensity:MaterialParameter = 0.25;
float _WindBFrequency:MaterialParameter = 7.7;
float3 _WindBTiling:MaterialParameter = {0.37,3,0};
float3 _WindBWrap:MaterialParameter = {0.5,0.5,0};
float _WindCIntensity:MaterialParameter = 0.125;
float _WindCFrequency:MaterialParameter = 11.7;
float3 _WindCTiling:MaterialParameter = {0.77,3,0};
float3 _WindCWrap:MaterialParameter = {0.5,0.5,0};
float _RandomNormal:MaterialParameter = 0.15;
float4 _PivotPosWS:MaterialParameter = {0,0,0,0};
float3 _BoundSize:MaterialParameter = {1,1,0};

sampler _BaseColorTextureSampler = sampler_state
{
    Texture = <_BaseColorTexture>;
    MipFilter = LINEAR;
    MinFilter = LINEAR;
    MagFilter = LINEAR;
};

// https://github.com/ColinLeung-NiloCat/UnityURP-MobileDrawMeshInstancedIndirectExample/blob/master/Assets/URPMobileGrassInstancedIndirectDemo/InstancedIndirectGrass/Core/InstancedIndirectGrass.shader
void TransformGrassPos(VS_INPUT In, out float3 perGrassPivotPosWS, out float3 positionOS, out float3 positionWS, out float wind, out float3 cameraTransformRightWS, out float3 cameraTransformUpWS, out float3 cameraTransformForwardWS, out float3 N)
{
	perGrassPivotPosWS = mul(In.PosInst, g_World).xyz;//we pre-transform to posWS in C# now

	float perGrassHeight = lerp(2,5,(sin(perGrassPivotPosWS.x*23.4643 + perGrassPivotPosWS.z) * 0.45 + 0.55)) * _GrassHeight;

	// //get "is grass stepped" data(bending) from RT
	// float2 grassBendingUV = ((perGrassPivotPosWS.xz - _PivotPosWS.xz) / _BoundSize) * 0.5 + 0.5;//claculate where is this grass inside bound (can optimize to 2 MAD)
	// float stepped = tex2Dlod(_GrassBendingRT, float4(grassBendingUV, 0, 0)).x;

	//rotation(make grass LookAt() camera just like a billboard)
	//=========================================
	cameraTransformRightWS = float3(g_View[0][0],g_View[1][0],g_View[2][0]);//UNITY_MATRIX_V[0].xyz == world space camera Right unit vector
	cameraTransformUpWS = float3(g_View[0][1],g_View[1][1],g_View[2][1]);//UNITY_MATRIX_V[1].xyz == world space camera Up unit vector
	cameraTransformForwardWS = float3(g_View[0][2],g_View[1][2],g_View[2][2]);//UNITY_MATRIX_V[2].xyz == -1 * world space camera Forward unit vector

	//Expand Billboard (billboard Left+right)
	positionOS = In.Pos.x * cameraTransformRightWS * _GrassWidth * (sin(perGrassPivotPosWS.x*95.4643 + perGrassPivotPosWS.z) * 0.45 + 0.55);//random width from posXZ, min 0.1

	//Expand Billboard (billboard Up)
	positionOS += In.Pos.y * cameraTransformUpWS;         
	//=========================================


	//bending by RT (hard code)
	float3 bendDir = cameraTransformForwardWS;
	bendDir.xz *= 0.5; //make grass shorter when bending, looks better
	bendDir.y = min(-0.5,bendDir.y);//prevent grass become too long if camera forward is / near parallel to ground
	// positionOS = lerp(positionOS.xyz + bendDir * positionOS.y / -bendDir.y, positionOS.xyz, stepped * 0.95 + 0.05);//don't fully bend, will produce ZFighting

	//per grass height scale
	positionOS.y *= perGrassHeight;

	// //camera distance scale (make grass width larger if grass is far away to camera, to hide smaller than pixel size triangle flicker)        
	// float3 viewWS = g_Eye - perGrassPivotPosWS;
	// float ViewWSLength = length(viewWS);
	// positionOS += cameraTransformRightWS * In.Pos.x * max(0, ViewWSLength * 0.0225);


	//move grass posOS -> posWS
	positionWS = positionOS + perGrassPivotPosWS;

	//wind animation (biilboard Left Right direction only sin wave)            
	wind = 0;
	wind += (sin(g_Time * _WindAFrequency + perGrassPivotPosWS.x * _WindATiling.x + perGrassPivotPosWS.z * _WindATiling.y)*_WindAWrap.x+_WindAWrap.y) * _WindAIntensity; //windA
	wind += (sin(g_Time * _WindBFrequency + perGrassPivotPosWS.x * _WindBTiling.x + perGrassPivotPosWS.z * _WindBTiling.y)*_WindBWrap.x+_WindBWrap.y) * _WindBIntensity; //windB
	wind += (sin(g_Time * _WindCFrequency + perGrassPivotPosWS.x * _WindCTiling.x + perGrassPivotPosWS.z * _WindCTiling.y)*_WindCWrap.x+_WindCWrap.y) * _WindCIntensity; //windC
	wind *= In.Pos.y; //wind only affect top region, don't affect root region
	float3 windOffset = cameraTransformRightWS * wind; //swing using billboard left right direction
	positionWS.xyz += windOffset;

	float3 randomAddToN = (_RandomNormal* sin(perGrassPivotPosWS.x * 82.32523 + perGrassPivotPosWS.z) + wind * -0.25) * cameraTransformRightWS;//random normal per grass 
	//default grass's normal is pointing 100% upward in world space, it is an important but simple grass normal trick
	//-apply random to normal else lighting is too uniform
	//-apply cameraTransformForwardWS to normal because grass is billboard
	N = normalize(half3(0,1,0) + randomAddToN + cameraTransformForwardWS*0.5);
}

struct NORMAL_VS_OUTPUT
{
	float4 Pos				: SV_Position;
	float3 Normal			: NORMAL;
	float3 PosVS			: TEXCOORD0;
};

NORMAL_VS_OUTPUT NormalVS( VS_INPUT In )
{
	NORMAL_VS_OUTPUT Output;

	float3 perGrassPivotPosWS, positionOS, positionWS, cameraTransformRightWS, cameraTransformUpWS, cameraTransformForwardWS, N; float wind;
	TransformGrassPos(In, perGrassPivotPosWS, positionOS, positionWS, wind, cameraTransformRightWS, cameraTransformUpWS, cameraTransformForwardWS, N);

	//vertex position logic done, complete posWS -> posCS
	//Output.positionCS = TransformWorldToHClip(positionWS);
	Output.Pos = mul(float4(positionWS,1), g_ViewProj);
	Output.PosVS = mul(float4(positionWS,1), g_View).xyz;
	Output.Normal = mul(N, (float3x3)g_View);
	return Output;
}

void NormalPS( 	NORMAL_VS_OUTPUT In,
				out float4 oNormal : COLOR0,
				out float4 oSpecular : COLOR1,
				out float4 oPos : COLOR2 )
{
    oNormal = float4(In.Normal, 1);
    oSpecular = float4(0.8, 0.3, 0, 1);
	oPos = float4(In.PosVS, 1.0);
}

struct OPAQUE_VS_OUTPUT
{
	float4 Pos				: SV_Position;
	float4 Color			: COLOR0;
	float3 ViewVS			: TEXCOORD1;
	float4 PosWS			: TEXCOORD2;
	float InvScreenDepth	: TEXCOORD3;
};

OPAQUE_VS_OUTPUT OpaqueVS( VS_INPUT In )
{
	OPAQUE_VS_OUTPUT Output;

	float3 perGrassPivotPosWS, positionOS, positionWS, cameraTransformRightWS, cameraTransformUpWS, cameraTransformForwardWS, N; float wind;
	TransformGrassPos(In, perGrassPivotPosWS, positionOS, positionWS, wind, cameraTransformRightWS, cameraTransformUpWS, cameraTransformForwardWS, N);

	//vertex position logic done, complete posWS -> posCS
	//Output.positionCS = TransformWorldToHClip(positionWS);
	Output.PosWS = float4(positionWS,1);
	Output.Pos = mul(Output.PosWS, g_ViewProj);
    Output.InvScreenDepth = Output.Pos.w / Output.Pos.z;
    float3 View = normalize(positionWS - g_Eye);
	Output.ViewVS = mul(View, (float3x3)g_View);

	// /////////////////////////////////////////////////////////////////////
	// //lighting & color
	// /////////////////////////////////////////////////////////////////////

	// //lighting data
	// Light mainLight;
	// #if _MAIN_LIGHT_SHADOWS
	// mainLight = GetMainLight(TransformWorldToShadowCoord(positionWS));
	// #else
	// mainLight = GetMainLight();
	// #endif
	// half3 randomAddToN = (_RandomNormal* sin(perGrassPivotPosWS.x * 82.32523 + perGrassPivotPosWS.z) + wind * -0.25) * cameraTransformRightWS;//random normal per grass 
	// //default grass's normal is pointing 100% upward in world space, it is an important but simple grass normal trick
	// //-apply random to normal else lighting is too uniform
	// //-apply cameraTransformForwardWS to normal because grass is billboard
	// half3 N = normalize(half3(0,1,0) + randomAddToN - cameraTransformForwardWS*0.5);

	// half3 V = viewWS / ViewWSLength;

	float3 baseColor = tex2Dlod(_BaseColorTextureSampler, float4(perGrassPivotPosWS.xz*float2(0.02,0.02)+float2(-0.43,-1.87),0,0)).rgb;//sample mip 0 only
	// half3 albedo = lerp(_GroundColor,baseColor, IN.positionOS.y);

	// //indirect
	// half3 lightingResult = SampleSH(0) * albedo;

	// //main direct light
	// lightingResult += ApplySingleDirectLight(mainLight, N, V, albedo, positionOS.y);

	// // Additional lights loop
	// #if _ADDITIONAL_LIGHTS

	// // Returns the amount of lights affecting the object being renderer.
	// // These lights are culled per-object in the forward renderer
	// int additionalLightsCount = GetAdditionalLightsCount();
	// for (int i = 0; i < additionalLightsCount; ++i)
	// {
		// // Similar to GetMainLight, but it takes a for-loop index. This figures out the
		// // per-object light index and samples the light buffer accordingly to initialized the
		// // Light struct. If _ADDITIONAL_LIGHT_SHADOWS is defined it will also compute shadows.
		// Light light = GetAdditionalLight(i, positionWS);

		// // Same functions used to shade the main light.
		// lightingResult += ApplySingleDirectLight(light, N, V, albedo, positionOS.y);
	// }
	// #endif

	// //fog
	// float fogFactor = ComputeFogFactor(Output.positionCS.z);
	// // Mix the pixel color with fogColor. You can optionaly use MixFogColor to override the fogColor
	// // with a custom one.
	// Output.color = MixFog(lightingResult, fogFactor);
	Output.Color = float4(baseColor,In.Pos.y*0.3);

	return Output;
}

float4 OpaquePS( OPAQUE_VS_OUTPUT In ) : COLOR0
{ 
	float3 SkyLightDirVS = mul(g_SkyLightDir, (float3x3)g_View);
    float3 NormalVS = tex2D(NormalRTSampler, (In.Pos.xy + 0.5f) / g_ScreenDim).xyz;
    float SkyLightAmount = saturate(GetLigthAmount(In.PosWS, In.InvScreenDepth) * dot(NormalVS, SkyLightDirVS));
    float3 SkyDiffuse = g_SkyLightColor.xyz * SkyLightAmount;
    float3 Ref = reflect(In.ViewVS, NormalVS);
    float SkySpecular = DistributionGGX(SkyLightDirVS, Ref, 0.8) * g_SkyLightColor.w * 0.3 * SkyLightAmount;
    float4 ScreenLight = tex2D(LightRTSampler, (In.Pos.xy + 0.5f) / g_ScreenDim);
	float3 Final = In.Color.xyz * (ScreenLight.xyz + SkyDiffuse) + ScreenLight.w + SkySpecular;
	return float4(Final, 1);
}

technique RenderScene
{
    pass PassTypeShadow
    {          
    }
    pass PassTypeNormal
    {          
        VertexShader = compile vs_3_0 NormalVS();
        PixelShader  = compile ps_3_0 NormalPS(); 
    }
    pass PassTypeNormalTrans
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
        VertexShader = compile vs_3_0 OpaqueVS();
        PixelShader  = compile ps_3_0 OpaquePS(); 
    }
    pass PassTypeTransparent
    {          
    }
}
