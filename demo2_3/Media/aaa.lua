
material=Material()
material.Shader="particle1.fx"
material.PassMask=Material.PassMaskTransparent
material:AddParameter("g_MeshTexture", ParameterValueTexture("texture/flare.dds"))
game:SaveMaterial("material/particle1.xml", material)

material=Material()
material.Shader="light1.fx"
material.PassMask=Material.PassMaskLight
material:AddParameter("g_MeshTexture", ParameterValueTexture("texture/white.bmp"))
game:SaveMaterial("material/light1.xml", material)

material=Material()
material.Shader="lambert1.fx"
material.PassMask=Material.PassMaskOpaque
material:AddParameter("g_MeshTexture", ParameterValueTexture("texture/casual19_m_35.jpg"))
material:AddParameter("g_NormalTexture", ParameterValueTexture("texture/casual19_m_35_normal.dds"))
material:AddParameter("g_SpecularTexture", ParameterValueTexture("texture/casual19_m_35_spec.dds"))
game:SaveMaterial("material/casual19_m_highpolyPhong.xml", material)

material=Material()
material.Shader="lambert1.fx"
material.PassMask=Material.PassMaskOpaque
material:AddParameter("g_MeshTexture", ParameterValueTexture("texture/sportive03_f_30_hair.jpg"))
material:AddParameter("g_NormalTexture", ParameterValueTexture("texture/sportive03_f_30_hair_normal.png"))
material:AddParameter("g_SpecularTexture", ParameterValueTexture("texture/sportive03_f_30_hair_spec.png"))
game:SaveMaterial("material/sportive03_f_highpoly_hairPhong.xml", material)

material=Material()
material.Shader="lambert1.fx"
material.PassMask=Material.PassMaskOpaque
material:AddParameter("g_MeshTexture", ParameterValueTexture("texture/sportive03_f_30.jpg"))
material:AddParameter("g_NormalTexture", ParameterValueTexture("texture/sportive03_f_30_normal.png"))
material:AddParameter("g_SpecularTexture", ParameterValueTexture("texture/sportive03_f_30_spec.png"))
game:SaveMaterial("material/sportive03_f_highpolyPhong.xml", material)

material=Material()
material.Shader="lambert1.fx"
material.PassMask=Material.PassMaskOpaque
material:AddParameter("g_MeshTexture", ParameterValueTexture("texture/wall.jpg"))
material:AddParameter("g_NormalTexture", ParameterValueTexture("texture/wall_NM_height.DDS"))
material:AddParameter("g_SpecularTexture", ParameterValueTexture("texture/white.bmp"))
game:SaveMaterial("material/lambert1.xml", material)
game:SaveMaterial("material/lambert2.xml", material)
game:SaveMaterial("material/lambert3.xml", material)
game:SaveMaterial("material/lambert4.xml", material)
game:SaveMaterial("material/lambert5.xml", material)

emitter=SphericalEmitter()
emitter.SpawnInterval=1/100
emitter.ParticleLifeTime=10
emitter.SpawnSpeed=5
emitter.SpawnInclination:AddNode(0,math.rad(45),0,0)
local Azimuth=math.rad(360)*8
emitter.SpawnAzimuth:AddNode(0,0,Azimuth/10,Azimuth/10)
emitter.SpawnAzimuth:AddNode(10,Azimuth,Azimuth/10,Azimuth/10)
emitter.SpawnColorA:AddNode(0,255,0,0)
emitter.SpawnColorA:AddNode(10,0,0,0)
emitter.SpawnColorR:AddNode(0,255,0,0)
emitter.SpawnColorR:AddNode(10,0,0,0)
emitter.SpawnColorG:AddNode(0,255,0,0)
emitter.SpawnColorG:AddNode(10,0,0,0)
emitter.SpawnColorB:AddNode(0,255,0,0)
emitter.SpawnColorB:AddNode(10,0,0,0)
emitter.SpawnSizeX:AddNode(0,1,0,0)
emitter.SpawnSizeX:AddNode(10,10,0,0)
emitter.SpawnSizeY:AddNode(0,1,0,0)
emitter.SpawnSizeY:AddNode(10,10,0,0)
game:SaveEmitter("emitter/emitter_01.xml", emitter)
