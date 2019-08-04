
particle1=Material()
particle1.Shader="mtl_particle1.fx"
particle1.PassMask=Material.PassMaskTransparent
particle1.MeshTexture.Path="texture/flare.dds"
game:SaveMaterial(particle1, "material/particle1.xml");

light1=Material()
light1.Shader="mtl_light1.fx"
light1.PassMask=Material.PassMaskLight
light1.MeshTexture.Path="texture/White.dds"
game:SaveMaterial(light1, "material/light1.xml");

casual19_m_35=Material()
casual19_m_35.Shader="mtl_lambert1.fx"
casual19_m_35.PassMask=Material.PassMaskOpaque
casual19_m_35.MeshTexture.Path="texture/casual19_m_35.jpg"
casual19_m_35.NormalTexture.Path="texture/casual19_m_35_normal.png"
casual19_m_35.SpecularTexture.Path="texture/casual19_m_35_spec.png"
game:SaveMaterial(casual19_m_35, "material/casual19_m_highpolyPhong.xml");

lambert1=Material()
lambert1.Shader="mtl_lambert1.fx"
lambert1.PassMask=Material.PassMaskOpaque
lambert1.MeshTexture.Path="texture/Checker.bmp"
lambert1.NormalTexture.Path="texture/Normal.dds"
lambert1.SpecularTexture.Path="texture/White.dds"
game:SaveMaterial(lambert1, "material/lambert1.xml");

local cmp = SphericalEmitterComponent()
cmp.SpawnInterval=1/100
cmp.ParticleLifeTime=10
cmp.SpawnSpeed=5
cmp.SpawnInclination:AddNode(0,math.rad(45),0,0)
local Azimuth=math.rad(360)*8
cmp.SpawnAzimuth:AddNode(0,0,Azimuth/10,Azimuth/10)
cmp.SpawnAzimuth:AddNode(10,Azimuth,Azimuth/10,Azimuth/10)
cmp.SpawnColorA:AddNode(0,1,0,0)
cmp.SpawnColorA:AddNode(10,0,0,0)
cmp.SpawnColorR:AddNode(0,1,0,0)
cmp.SpawnColorR:AddNode(10,0,0,0)
cmp.SpawnColorG:AddNode(0,1,0,0)
cmp.SpawnColorG:AddNode(10,0,0,0)
cmp.SpawnColorB:AddNode(0,1,0,0)
cmp.SpawnColorB:AddNode(10,0,0,0)
cmp.SpawnSizeX:AddNode(0,1,0,0)
cmp.SpawnSizeX:AddNode(10,10,0,0)
cmp.SpawnSizeY:AddNode(0,1,0,0)
cmp.SpawnSizeY:AddNode(10,10,0,0)
cmp.Material = particle1
game:SaveComponent(cmp,"emitter/emitter_01_cmp.xml")
