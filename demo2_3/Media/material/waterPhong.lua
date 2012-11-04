function SetupMaterial(material)
	material.Effect = game:LoadEffect("WaterEffect.fx")
	material:BeginParameterBlock("WaterEffect")
	material.Effect:SetTexture("g_WaterNormalTexture", game:LoadTexture("WaterNormal2.png"))
	material.Effect:SetTexture("g_CubeTexture", game:LoadCubeTexture("galileo_cross.dds"))
	material:EndParameterBlock()
end