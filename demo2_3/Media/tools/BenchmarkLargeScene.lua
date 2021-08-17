collectionObjs=CollectionObjMap()
acts={}
local terrain=Terrain(NamedObject.MakeUniqueName("editor_terrain"),1,1,32,2)
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
-- act:CreateRigidActor(Actor.eRIGID_STATIC)
-- terrain:CreateHeightFieldShape(false,collectionObjs)
-- terrain.SimulationFilterWord0=1
-- terrain.QueryFilterWord0=1
act:UpdateWorld()

-- local emit_cmp=StaticEmitter(NamedObject.MakeUniqueName("editor_emitter"),act.aabb:shrink(0,-100,0),4,
	-- EmitterComponent.FaceTypeCamera,EmitterComponent.SpaceTypeLocal,EmitterComponent.VelocityTypeNone,EmitterComponent.PrimitiveTypeQuad)
-- emit_cmp.Material=Material()
-- emit_cmp.Material.Shader="shader/mtl_BlinnPhong.fx"
-- emit_cmp.Material:ParseShaderParameters()
-- emit_cmp.EmitterChunkPath="terrain/emit_123456"
-- act:AddComponent(emit_cmp)
-- local estr=StaticEmitterStream(emit_cmp)
-- for i=1,300000,1 do
	-- estr:Spawn(Vector3(math.random(act.aabb.min.x,act.aabb.max.x),
		-- 0,math.random(act.aabb.min.x,act.aabb.max.x)),Vector3(0,0,0),Vector4(1,1,1,1),Vector2(10,10),0,0)
-- end
-- estr:Release()

-- act.LodFactor=1.5
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
	local act=Actor(NamedObject.MakeUniqueName("editor_act"),Vector3(
		math.random(act.OctAabb.min.x,act.OctAabb.max.x),
		math.random(0,100),
		math.random(act.OctAabb.min.x,act.OctAabb.max.x)),Quaternion.Identity(),Vector3(1,1,1),AABB(-1,1))
	act:AddComponent(mesh_cmp)
	-- act:CreateRigidActor(Actor.eRIGID_STATIC)
	-- mesh_cmp:CreateTriangleMeshShape(false,collectionObjs)
	-- mesh_cmp.SimulationFilterWord0=1
	-- mesh_cmp.QueryFilterWord0=1
	act.CullingDist=500
	act:UpdateWorld()
	theApp.MainWnd:AddEntity(act2entity(act),act.aabb:transform(act.World),1.0,0.1)
	table.insert(acts,act)
end
