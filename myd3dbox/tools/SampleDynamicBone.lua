local mesh_cmp=MeshComponent(NamedObject.MakeUniqueName("mesh_cmp"))
mesh_cmp.MeshPath="mesh/Cloth.mesh.xml"
mesh_cmp.MeshSubMeshId=0
mesh_cmp.Material=Material()
mesh_cmp.Material.Shader="shader/mtl_BlinnPhong.fx"
mesh_cmp.Material:ParseShaderParameters()

local anim_cmp=Animator(NamedObject.MakeUniqueName("anim_cmp"))
anim_cmp.SkeletonPath="mesh/Cloth.skeleton.xml"
anim_cmp.Child0=AnimationNodeSequence("clip1")
anim_cmp:ReloadSequenceGroup()

act=Actor(NamedObject.MakeUniqueName("actor"),Vector3(0,0,0),Quaternion.Identity(),Vector3(1,1,1),AABB(-1,1))
act:InsertComponent(mesh_cmp)
act:InsertComponent(anim_cmp)
theApp.MainWnd:AddEntity(act,act.aabb:transform(act.World),1.0,0.1)

local skel=theApp:LoadSkeleton(anim_cmp.SkeletonPath)
anim_cmp:AddDynamicBone(skel:GetBoneIndex("joint2"),skel.boneHierarchy,0.1,0.001,-10)
