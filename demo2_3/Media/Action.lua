module("SAction", package.seeall)

skel=client:LoadSkeleton("character/jack.skeleton.xml")
-- skel:AddOgreSkeletonAnimationFromFile("character/casual19_m_highpoly_idle1.skeleton.xml")
-- skel:AddOgreSkeletonAnimationFromFile("character/casual19_m_highpoly_run.skeleton.xml")
-- skel:AddOgreSkeletonAnimationFromFile("character/casual19_m_highpoly_walk.skeleton.xml")
-- skel:AddOgreSkeletonAnimationFromFile("character/casual19_m_highpoly_jumpforward.skeleton.xml")
-- skel:AdjustAnimationRoot(Bone(Vector3(0,-95,0)))
-- skel:SaveOgreSkeletonAnimation("Media/character/casual19_m_highpoly_full.skeleton.xml")

act_jump=Action(5)
local track=ActionTrackAnimation()
track:AddKeyFrame(0,"node_run_slot","clip_jump",2.0,1.0,0.3,0.3,"",1)
act_jump:AddTrack(track)

act_sound=Action(9999)
local track=ActionTrackSound()
track:AddKeyFrame(0,"sound/drumloop.wav",0,999,true,1,5)
act_sound:AddTrack(track)

act_tuowei=Action(9999)
local particle1=Material()
particle1.Shader="shader/mtl_Particle.fx"
particle1.PassMask=Material.PassMaskTransparent
particle1.ZWriteEnable=false
particle1.BlendMode=Material.BlendModeAdditive
particle1:SetParameter("g_Texture", "texture/flare.dds")
act_tuowei_emit=SphericalEmitter(NamedObject.MakeUniqueName("act_tuowei_emit"),1024,EmitterComponent.FaceTypeCamera,EmitterComponent.SpaceTypeWorld)
act_tuowei_emit.SpawnInterval=0.1
act_tuowei_emit.ParticleLifeTime=5
act_tuowei_emit.ParticleColorR:AddNode(0,1,0,0)
act_tuowei_emit.ParticleColorR:AddNode(3,0,0,0)
act_tuowei_emit.ParticleColorG:AddNode(0,1,0,0)
act_tuowei_emit.ParticleColorG:AddNode(3,0,0,0)
act_tuowei_emit.ParticleColorB:AddNode(0,1,0,0)
act_tuowei_emit.ParticleColorB:AddNode(3,0,0,0)
act_tuowei_emit.ParticleColorA:AddNode(0,1,0,0)
act_tuowei_emit.ParticleColorA:AddNode(3,0,0,0)
act_tuowei_emit.ParticleSizeX:AddNode(0,1,0,0)
act_tuowei_emit.ParticleSizeY:AddNode(0,1,0,0)
act_tuowei_emit.SpawnBoneId=skel:GetBoneIndex("joint6")
act_tuowei_emit.Material=particle1
local track=ActionTrackEmitter()
track:AddKeyFrame(0,99999,act_tuowei_emit.Name)
act_tuowei:AddTrack(track)

act_pose=Action(100)
act_pose_track0=ActionTrackPose()
for t=0,100,6 do
    act_pose_track0:AddKeyFrame(t,3)
end
act_pose_track0.ParamPose=Bone(Vector3(-3,1,3-5))
act_pose:AddTrack(act_pose_track0)
act_pose_track1=ActionTrackPose()
for t=3,100,6 do
    act_pose_track1:AddKeyFrame(t,3)
end
act_pose_track1.ParamPose=Bone(Vector3(-3,1,-3-5))
act_pose:AddTrack(act_pose_track1)
