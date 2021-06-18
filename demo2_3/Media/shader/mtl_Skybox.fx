
texture g_DiffuseTexture:MaterialParameter<string Initialize="texture/Checker.bmp";>;

sampler DiffuseTextureSampler = sampler_state
{
    Texture = <g_DiffuseTexture>;
    MipFilter = LINEAR;
    MinFilter = LINEAR;
    MagFilter = LINEAR;
    ADDRESSU = WRAP;
    ADDRESSV = WRAP;
};

struct SKYBOX_VS_OUTPUT
{
	float4 Pos				: SV_Position;
	float4 Color			: COLOR0;
	float2 Tex0				: TEXCOORD0;
};

SKYBOX_VS_OUTPUT SkyboxVS( VS_INPUT In )
{
    SKYBOX_VS_OUTPUT Output;
	Output.Pos = mul(float4(In.Pos.xyz + g_Eye,1), g_ViewProj);
	Output.Color = TransformColor(In);
	Output.Tex0 = TransformUV(In);
    return Output;
}

float4 SkyboxPS( SKYBOX_VS_OUTPUT In ) : COLOR0
{ 
	return tex2D(DiffuseTextureSampler, In.Tex0);
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
        VertexShader = compile vs_3_0 SkyboxVS();
        PixelShader  = compile ps_3_0 SkyboxPS(); 
	}
    pass PassTypeOpaque
    {          
    }
    pass PassTypeTransparent
    {          
    }
}
