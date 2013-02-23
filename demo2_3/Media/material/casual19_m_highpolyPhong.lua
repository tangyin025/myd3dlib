function SetupMaterial(material)
	material.Effect = game:LoadEffect("shader/Character.fx")
	material:SetVector("g_MaterialAmbientColor", Vector4(0.3,0.3,0.3,1.0))
	material:SetVector("g_MaterialDiffuseColor", Vector4(1.0,1.0,1.0,1.0))
	material:SetTexture("g_MeshTexture", game:LoadTexture("texture/casual19_m_35.jpg"))
	material:SetTexture("g_NormalTexture", game:LoadTexture("texture/casual19_m_35_normal.png"))
	material:SetTexture("g_SpecularTexture", game:LoadTexture("texture/casual19_m_35_spec.png"))
	material:SetTexture("g_CubeTexture", game:LoadCubeTexture("texture/galileo_cross.dds"))
end