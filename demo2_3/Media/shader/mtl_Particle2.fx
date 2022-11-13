
texture g_Texture:MaterialParameter<string path="texture/Checker.bmp";>;
float2 g_Tiles:MaterialParameter = float2(4, 4);
float2 g_TileAreas:MaterialParameter = float2(1, 1);

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
    float TotalFrames = fmod(Output.Color.x * 255.0, g_Tiles.x * g_Tiles.y);
    float Row = floor(TotalFrames / g_Tiles.x);
    float Frame = fmod(TotalFrames, g_Tiles.x);
    float2 Tex = TransformUV(In);
	Output.Tex0.x = (Frame + Tex.x) / g_Tiles.x * g_TileAreas.x;
    Output.Tex0.y = (Row + Tex.y) / g_Tiles.y * g_TileAreas.y;
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
