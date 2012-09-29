function SetupMaterial(material)
	material.Effect = Loader.LoadEffect("WaterEffect.fx")
	material:BeginParameterBlock("WaterEffect")
	material.Effect:SetTexture("g_WaterNormalTexture", Loader.LoadTexture("WaterNormal2.png"))
	material.Effect:SetTexture("g_CubeTexture", Loader.LoadCubeTexture("galileo_cross.dds"))
	material:EndParameterBlock()
end