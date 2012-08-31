local function LoadTexture(name)
	local texture = game:LoadTexture(name)
	table.insert(texture_pool, texture)
	return texture
end

function SetupMaterial(material)
	local effect = game:LoadEffect("SkinedMesh.fx")
	material.Effect = effect
	effect:SetTechnique("RenderScene")
	material:BeginParameterBlock()
	effect:SetVector("g_MaterialAmbientColor", Vector4(0.3,0.3,0.3,0.3))
	effect:SetVector("g_MaterialDiffuseColor", Vector4(1.0,1.0,1.0,1.0))
	effect:SetTexture("g_MeshTexture", LoadTexture("casual19_m_35.jpg"))
	effect:SetTexture("g_txNormalMap", LoadTexture("casual19_m_35_normal.png"))
	effect:SetTexture("g_txSpecularMap", LoadTexture("casual19_m_35_spec.png"))
	material:EndParameterBlock()
end