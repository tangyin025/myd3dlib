acts={}
local terrain=Terrain(NamedObject.MakeUniqueName("editor_terrain"),64,64,128,0.01)
terrain.ChunkPath="terrain/chunk_123456"
local tstr=TerrainStream(terrain)
tstr:GetPos(0,0)
tstr:Release()
terrain.Material=Material()
terrain.Material.Shader="shader/mtl_BlinnPhong.fx"
terrain.Material:ParseShaderParameters()
local act=Actor(NamedObject.MakeUniqueName("editor_act"),Vector3(-4096,0,-4096),Quaternion.Identity(),Vector3(1,1,1),AABB(0,-4096,0,8192,4096,8192))
act:AddComponent(terrain)
act.LodFactor=1.5
act:UpdateWorld()
theApp.MainWnd:AddEntity(actor2ent(act),act.aabb:transform(act.World),1.0,0.1)
table.insert(acts,act)

for i=1,300000,1 do
	local mesh_cmp=MeshComponent(NamedObject.MakeUniqueName("editor_mesh_cmp"))
	mesh_cmp.MeshPath="mesh/Gear.mesh.xml"
	mesh_cmp.MeshSubMeshId=0
	mesh_cmp.Material=Material()
	mesh_cmp.Material.Shader="shader/mtl_BlinnPhong.fx"
	mesh_cmp.Material:ParseShaderParameters()
	local act=Actor(NamedObject.MakeUniqueName("editor_act"),
		Vector3(math.random(-4096,4096),math.random(0,100),math.random(-4096,4096)),Quaternion.Identity(),Vector3(1,1,1),AABB(-1,1))
	act:AddComponent(mesh_cmp)
	act.CullingDist=500
	act:UpdateWorld()
	theApp.MainWnd:AddEntity(actor2ent(act),act.aabb:transform(act.World),1.0,0.1)
	table.insert(acts,act)
end
