function SetupMaterial(material)
	material.Effect = game:LoadEffect("WaterEffect.fx")
	material:SetTexture("g_WaterNormalTexture", game:LoadTexture("WaterNormal2.png"))
	material:SetTexture("g_CubeTexture", game:LoadCubeTexture("galileo_cross.dds"))
end