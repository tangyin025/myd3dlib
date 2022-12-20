
texture g_Texture:MaterialParameter<string path="texture/Checker.bmp";>;
float2 g_TileSize:MaterialParameter = float2(1,1);

sampler TextureSampler = sampler_state
{
    Texture = <g_Texture>;
    MipFilter = LINEAR;
    MinFilter = LINEAR;
    MagFilter = LINEAR;
};

struct TRANSPARENT_VS_OUTPUT
{
	float4 Pos				: POSITION;
	float2 Tex0				: TEXCOORD0;
	float4 Color			: COLOR0;
};

TRANSPARENT_VS_OUTPUT TransparentVS( VS_INPUT In )
{
    TRANSPARENT_VS_OUTPUT Output;
	Output.Pos = TransformPos(In);
	Output.Color = TransformColor(In);
	Output.Tex0 = TileUV(TransformUV(In), g_TileSize, Output.Color.r * 255);
    return Output;    
}

float4 TransparentPS( TRANSPARENT_VS_OUTPUT In ) : COLOR0
{ 
	return tex2D(TextureSampler, In.Tex0);
}

technique RenderScene
{
    pass PassTypeShadow
    {          
    }
    pass PassTypeNormal
    {          
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
    }
    pass PassTypeTransparent
    {          
		VertexShader = compile vs_3_0 TransparentVS();
		PixelShader  = compile ps_3_0 TransparentPS();
    }
}
