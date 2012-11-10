function SetupMaterial(material)
	material.Effect = game:LoadEffect("Character.fx")
	material:AddVector("g_MaterialAmbientColor", Vector4(0.3,0.3,0.3,1.0))
	material:AddVector("g_MaterialDiffuseColor", Vector4(1.0,1.0,1.0,1.0))
	material:AddTexture("g_MeshTexture", game:LoadTexture("casual19_m_35.jpg"))
	material:AddTexture("g_NormalTexture", game:LoadTexture("casual19_m_35_normal.png"))
	material:AddTexture("g_SpecularTexture", game:LoadTexture("casual19_m_35_spec.png"))
	material:AddTexture("g_CubeTexture", game:LoadCubeTexture("galileo_cross.dds"))
end