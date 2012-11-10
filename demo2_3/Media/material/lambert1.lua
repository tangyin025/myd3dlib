function SetupMaterial(material)
	material.Effect = game:LoadEffect("SimpleSample.fx")
	material:SetVector("g_MaterialAmbientColor", Vector4(0.3,0.3,0.3,1.0))
	material:SetVector("g_MaterialDiffuseColor", Vector4(1.0,1.0,1.0,1.0))
	material:SetTexture("g_MeshTexture", game:LoadTexture("Checker.bmp"))
end