
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

struct TRANSPARENT_VS_OUTPUT
{
	float4 Pos				: POSITION;
	float2 Tex0				: TEXCOORD0;
	float4 Color			: COLOR0;
};

TRANSPARENT_VS_OUTPUT VS( VS_INPUT In )
{
    TRANSPARENT_VS_OUTPUT Output;
	Output.Pos = TransformPos(In);
	Output.Tex0 = TransformUV(In);
	Output.Color = TransformColor(In);
    return Output;    
}

float4 PS( TRANSPARENT_VS_OUTPUT In ) : COLOR0
{ 
	float4 Diffuse = tex2D(DiffuseTextureSampler, In.Tex0);
	return Diffuse;
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
		VertexShader = compile vs_3_0 VS();
		PixelShader  = compile ps_3_0 PS();
    }
    pass PassTypeTransparent
    {          
    }
}
