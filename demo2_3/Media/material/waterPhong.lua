
local material=Material()
material.Effect = game:LoadEffect("shader/WaterEffect.fx")
material.ParameterMap:SetTexture("g_WaterNormalTexture", game:LoadTexture("texture/WaterNormal2.png"))
material.ParameterMap:SetTexture("g_CubeTexture", game:LoadCubeTexture("texture/galileo_cross.dds"))
game:InsertMaterial("waterPhong", material)