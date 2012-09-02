function SetupMaterial(material)
	material.Effect = LoadEffect("WaterEffect.fx")
	material:BeginParameterBlock("WaterEffect")
	material.Effect:SetTexture("g_WaterNormalTexture", LoadTexture("WaterNormal2.png"))
	material:EndParameterBlock()
end