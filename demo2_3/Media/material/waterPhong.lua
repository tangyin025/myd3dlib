function SetupMaterial(material)
	material.Effect = game:LoadEffect("shader/WaterEffect.fx")
	material:SetTexture("g_WaterNormalTexture", game:LoadTexture("texture/WaterNormal2.png"))
	material:SetTexture("g_CubeTexture", game:LoadCubeTexture("texture/galileo_cross.dds"))
end