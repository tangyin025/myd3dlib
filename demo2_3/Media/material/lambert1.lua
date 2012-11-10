function SetupMaterial(material)
	material.Effect = game:LoadEffect("SimpleSample.fx")
	material:AddVector("g_MaterialAmbientColor", Vector4(0.3,0.3,0.3,1.0))
	material:AddVector("g_MaterialDiffuseColor", Vector4(1.0,1.0,1.0,1.0))
	material:AddTexture("g_MeshTexture", game:LoadTexture("Checker.bmp"))
end