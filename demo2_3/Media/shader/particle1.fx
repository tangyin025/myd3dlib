
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
	Output.Tex0 = TransformUV(In);
	Output.Color = TransformColor(In);
    return Output;    
}

float4 TransparentPS( TRANSPARENT_VS_OUTPUT In ) : COLOR0
{ 
	return tex2D(MeshTextureSampler, In.Tex0) * In.Color;
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
