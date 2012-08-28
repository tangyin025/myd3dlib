function SetupMaterial(material)
	local effect = game:LoadEffect("SkinedMesh.fx")
	material.Effect = effect
	local texture = game:LoadTexture("casual19_m_35.jpg")
	table.insert(texture_pool, texture)
	effect:SetTechnique("RenderScene")
	material:BeginParameterBlock()
	effect:SetVector("g_MaterialAmbientColor", Vector4(1,1,1,1))
	effect:SetVector("g_MaterialDiffuseColor", Vector4(1,1,1,1))
	effect:SetTexture("g_MeshTexture", texture)
	material:EndParameterBlock()
end