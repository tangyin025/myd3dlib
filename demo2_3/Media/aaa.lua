
emitter=SphericalEmitter()
emitter.SpawnInterval=1/100
emitter.ParticleLifeTime=10
emitter.SpawnSpeed=5
emitter.SpawnInclination:AddNode(0,math.rad(45),0,0)
local Azimuth=math.rad(360)*8
emitter.SpawnAzimuth:AddNode(0,0,Azimuth/10,Azimuth/10)
emitter.SpawnAzimuth:AddNode(10,Azimuth,Azimuth/10,Azimuth/10)
emitter.ParticleColorA:AddNode(0,255,0,0)
emitter.ParticleColorA:AddNode(10,0,0,0)
emitter.ParticleColorR:AddNode(0,255,0,0)
emitter.ParticleColorR:AddNode(10,0,0,0)
emitter.ParticleColorG:AddNode(0,255,0,0)
emitter.ParticleColorG:AddNode(10,0,0,0)
emitter.ParticleColorB:AddNode(0,255,0,0)
emitter.ParticleColorB:AddNode(10,0,0,0)
emitter.ParticleSizeX:AddNode(0,1,0,0)
emitter.ParticleSizeX:AddNode(10,10,0,0)
emitter.ParticleSizeY:AddNode(0,1,0,0)
emitter.ParticleSizeY:AddNode(10,10,0,0)
emitter.ParticleAnimColumn=4
emitter.ParticleAnimRow=4
game:SaveEmitter("emitter/emitter_01.xml", emitter)

material=Material()
material.DiffuseTexture.first="texture/casual19_m_35.jpg"
material.NormalTexture.first="texture/casual19_m_35_normal.png"
material.SpecularTexture.first="texture/casual19_m_35_spec.png"
game:SaveMaterial("material/casual19_m_highpolyPhong.xml", material)

material=Material()
material.DiffuseTexture.first="texture/sportive03_f_30_hair.jpg"
material.NormalTexture.first="texture/sportive03_f_30_hair_normal.png"
material.SpecularTexture.first="texture/sportive03_f_30_hair_spec.png"
game:SaveMaterial("material/sportive03_f_highpoly_hairPhong.xml", material)

material=Material()
material.DiffuseTexture.first="texture/sportive03_f_30.jpg"
material.NormalTexture.first="texture/sportive03_f_30_normal.png"
material.SpecularTexture.first="texture/sportive03_f_30_spec.png"
game:SaveMaterial("material/sportive03_f_highpolyPhong.xml", material)

material=Material()
material.DiffuseTexture.first="texture/Checker.bmp"
-- material.NormalTexture.first="texture/casual19_m_35_normal.png"
-- material.SpecularTexture.first="texture/casual19_m_35_spec.png"
game:SaveMaterial("material/lambert1.xml", material)

-- material=Material()
-- material.Effect=game:LoadEffect("shader/WaterEffect.fx", EffectMacroPairList())
-- material:SetTexture("g_WaterNormalTexture",game:LoadTexture("texture/WaterNormal2.png")
-- material:SetTexture("g_CubeTexture",game:LoadTexture("texture/galileo_cross.dds"))
-- game:SaveMaterial("material/waterPhong.xml", material)

material=Material()
material.DiffuseTexture.first="texture/TileWall.dds"
material.NormalTexture.first="texture/casual19_m_35_normal.png"
material.SpecularTexture.first="texture/casual19_m_35_spec.png"
game:SaveMaterial("brick.xml", material)

material=Material()
material.DiffuseTexture.first="texture/Plaster_Tileable.dds"
material.NormalTexture.first="texture/casual19_m_35_normal.png"
material.SpecularTexture.first="texture/casual19_m_35_spec.png"
game:SaveMaterial("stone.xml", material)

-- game:InsertEmitter(game:LoadEmitter("emitter/emitter_01.xml"))