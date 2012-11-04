function SetupMaterial(material)
	material.Effect = game:LoadEffect("SimpleSample.fx")
	material:BeginParameterBlock("RenderScene")
	material.Effect:SetVector("g_MaterialAmbientColor", Vector4(0.3,0.3,0.3,1.0))
	material.Effect:SetVector("g_MaterialDiffuseColor", Vector4(1.0,1.0,1.0,1.0))
	material.Effect:SetTexture("g_MeshTexture", game:LoadTexture("Checker.bmp"))
	material:EndParameterBlock()
end