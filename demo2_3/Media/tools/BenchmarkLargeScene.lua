acts={}
local terrain=Terrain(NamedObject.MakeUniqueName("editor_terrain"),128,128,64,2)
terrain.ChunkPath="terrain/chunk_123456"
local tstr=TerrainStream(terrain)
tstr:GetPos(0,0)
tstr:Release()
terrain.Material=Material()
terrain.Material.Shader="shader/mtl_Splatmap.fx"
terrain.Material:ParseShaderParameters()
local tbox=terrain:CalculateAABB()
local act=Actor(NamedObject.MakeUniqueName("editor_act"),Vector3(-tbox:Center().x,0,-tbox:Center().z),Quaternion.Identity(),Vector3(1,1,1),tbox)
act:AddComponent(terrain)
act:CreateRigidActor(Actor.eRIGID_STATIC)
terrain:CreateHeightFieldShape(true)
terrain.SimulationFilterWord0=1
terrain.QueryFilterWord0=1
-- act.LodFactor=1.5
act:UpdateWorld()
theApp.MainWnd:AddEntity(act2entity(act),act.aabb:transform(act.World),1.0,0.1)
table.insert(acts,act)

local cache=theApp:LoadMesh("mesh/Gear.mesh.xml","")

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
	act:CreateRigidActor(Actor.eRIGID_STATIC)
	mesh_cmp:CreateTriangleMeshShape(true)
	mesh_cmp.SimulationFilterWord0=1
	mesh_cmp.QueryFilterWord0=1
	act.CullingDist=500
	act:UpdateWorld()
	theApp.MainWnd:AddEntity(act2entity(act),act.aabb:transform(act.World),1.0,0.1)
	table.insert(acts,act)
end
