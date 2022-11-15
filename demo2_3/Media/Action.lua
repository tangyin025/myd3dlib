module("SAction", package.seeall)

skel=client:LoadSkeleton("character/casual19_m_highpoly.skeleton.xml")
skel:AddOgreSkeletonAnimationFromFile("character/casual19_m_highpoly_idle1.skeleton.xml")
skel:AddOgreSkeletonAnimationFromFile("character/casual19_m_highpoly_run.skeleton.xml")
skel:AddOgreSkeletonAnimationFromFile("character/casual19_m_highpoly_walk.skeleton.xml")
skel:AddOgreSkeletonAnimationFromFile("character/casual19_m_highpoly_jumpforward.skeleton.xml")
skel:AdjustAnimationRoot(Bone(Vector3(0,-95,0)))
-- skel:SaveOgreSkeletonAnimation("Media/character/casual19_m_highpoly_full.skeleton.xml")

act_jump=Action()
local track=ActionTrackAnimation()
track:AddKeyFrame(0,"node_run_slot","jumpforward",2.0,1.0,0.3,0.3,false,1,"")
act_jump:AddTrack(track)

act_sound=Action()
local track=ActionTrackSound()
track:AddKeyFrame(0,"sound/drumloop.wav",true,1,5)
act_sound:AddTrack(track)

act_tuowei=Action()
local particle1=Material()
particle1.Shader="shader/mtl_Particle.fx"
particle1.PassMask=Material.PassMaskTransparent
particle1.ZWriteEnable=false
particle1.BlendMode=Material.BlendModeAdditive
particle1:AddParameter("g_Texture", "texture/flare.dds")
local track=ActionTrackEmitter()
track.EmitterSpaceType=EmitterComponent.SpaceTypeWorld
track.EmitterMaterial=particle1
track.ParticleLifeTime=5
track.ParticleColorR:AddNode(0,1,0,0)
track.ParticleColorR:AddNode(3,0,0,0)
track.ParticleColorG:AddNode(0,1,0,0)
track.ParticleColorG:AddNode(3,0,0,0)
track.ParticleColorB:AddNode(0,1,0,0)
track.ParticleColorB:AddNode(3,0,0,0)
track.ParticleColorA:AddNode(0,1,0,0)
track.ParticleColorA:AddNode(3,0,0,0)
track.SpawnBoneId=skel:GetBoneIndex("Bip01_Neck")
track:AddKeyFrame(0,99999,0.1)
act_tuowei:AddTrack(track)

act_pose=Action()
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

act_moving=Action()
act_moving_track=ActionTrackVelocity()
act_moving_track:AddKeyFrame(0,3)
act_moving_track.ParamVelocity=Vector3(1,1,1)
act_moving:AddTrack(act_moving_track)
