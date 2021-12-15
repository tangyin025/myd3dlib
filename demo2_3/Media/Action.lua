module("SAction", package.seeall)

act_jump=Action()
local track=ActionTrackAnimation()
track:AddKeyFrame(0,"jumpforward",2.0,1.0,0.3,0.3,false,1,"",-1)
act_jump:AddTrack(track)

act_sound=Action()
local track=ActionTrackSound()
track:AddKeyFrame(0,"sound/drumloop.wav",true,1,5)
act_sound:AddTrack(track)

act_tuowei=Action()
local particle1=Material()
particle1.Shader="shader/mtl_particle1.fx"
particle1.PassMask=Material.PassMaskTransparent
particle1.ZWriteEnable=false
particle1.BlendMode=Material.BlendModeAdditive
particle1:AddParameter("g_Texture", "texture/flare.dds")
local track=ActionTrackEmitter()
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
track.AttachBoneId=0--SPlayer.skel:GetBoneIndex("Bip01_Spine")
track:AddKeyFrame(0,99999,0.1)
act_tuowei:AddTrack(track)

act_pose=Action()
act_pose_track=ActionTrackPose(0,3,1)
act_pose_track:AddKeyFrame(0,Vector3(0,0,0))
act_pose_track:AddKeyFrame(3,Vector3(1,1,1))
act_pose:AddTrack(act_pose_track)

act_moving=Action()
act_moving_track=ActionTrackPose(0,10,9999)
act_moving_track:AddKeyFrame(0,Vector3(-3,1,3-5))
act_moving_track:AddKeyFrame(5,Vector3(-3,1,-3-5))
act_moving_track:AddKeyFrame(10,Vector3(-3,1,3-5))
act_moving:AddTrack(act_moving_track)
