function SetupMaterial(material)
	material.Effect = game:LoadEffect("WaterEffect.fx")
	material:AddTexture("g_WaterNormalTexture", game:LoadTexture("WaterNormal2.png"))
	material:AddTexture("g_CubeTexture", game:LoadCubeTexture("galileo_cross.dds"))
end