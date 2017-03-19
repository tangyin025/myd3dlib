
material=Material()
material.Shader="particle1.fx"
material.PassMask=Material.PassMaskTransparent
material.MeshTexture.Path="texture/flare.dds"
game:SaveMaterial(material, "material/particle1.xml");

material=Material()
material.Shader="light1.fx"
material.PassMask=Material.PassMaskLight
material.MeshTexture.Path="texture/White.dds"
game:SaveMaterial(material, "material/light1.xml");

material=Material()
material.Shader="lambert1.fx"
material.PassMask=Material.PassMaskOpaque
material.MeshTexture.Path="texture/casual19_m_35.jpg"
material.NormalTexture.Path="texture/casual19_m_35_normal.png"
material.SpecularTexture.Path="texture/casual19_m_35_spec.png"
game:SaveMaterial(material, "material/casual19_m_highpolyPhong.xml");

material=Material()
material.Shader="lambert1.fx"
material.PassMask=Material.PassMaskOpaque
material.MeshTexture.Path="texture/sportive03_f_30_hair.jpg"
material.NormalTexture.Path="texture/sportive03_f_30_hair_normal.png"
material.SpecularTexture.Path="texture/sportive03_f_30_hair_spec.png"
game:SaveMaterial(material, "material/sportive03_f_highpoly_hairPhong.xml");

material=Material()
material.Shader="lambert1.fx"
material.PassMask=Material.PassMaskOpaque
material.MeshTexture.Path="texture/sportive03_f_30.jpg"
material.NormalTexture.Path="texture/sportive03_f_30_normal.png"
material.SpecularTexture.Path="texture/sportive03_f_30_spec.png"
game:SaveMaterial(material, "material/sportive03_f_highpolyPhong.xml");

material=Material()
material.Shader="lambert1.fx"
material.PassMask=Material.PassMaskOpaque
material.MeshTexture.Path="texture/wall.jpg"
material.NormalTexture.Path="texture/wall_NM_height.DDS"
material.SpecularTexture.Path="texture/White.dds"
game:SaveMaterial(material, "material/lambert1.xml");
game:SaveMaterial(material, "material/lambert2.xml");
game:SaveMaterial(material, "material/lambert3.xml");
game:SaveMaterial(material, "material/lambert4.xml");
game:SaveMaterial(material, "material/lambert5.xml");

local cmp = SphericalEmitterComponent(Vector3(0,0,0),Quaternion(0,0,0,1),Vector3(1,1,1))
cmp.Emitter = Emitter()
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
local material=Material()
material.Shader="particle1.fx"
material.PassMask=Material.PassMaskTransparent
material.MeshTexture.Path="texture/flare.dds"
cmp.Material = material
game:SaveComponent(cmp,"emitter/emitter_01_cmp.xml")
