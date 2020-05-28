module("SAction", package.seeall)

act_jump=Action()
act_jump.Length=5.0
local track=ActionTrackAnimation()
track:AddKeyFrame(0,"jumpforward","",2.0,0.3,0.3,false,1,0,"")
act_jump:AddTrack(track)

act_sound=Action()
act_sound.Length=5.0
track=ActionTrackSound()
track:AddKeyFrame(0,"demo2_3/untitled/drumloop")
act_sound:AddTrack(track)

act_tuowei=Action()
act_tuowei.Length=99999
local particle1=Material()
particle1.Shader="shader/mtl_particle1.fx"
particle1.PassMask=Material.PassMaskTransparent
particle1.ZWriteEnable=0
particle1.BlendMode=Material.BlendModeAdditive
particle1:AddParameterTexture("g_Texture", "texture/flare.dds")
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
track.SpawnInterval=0.1
track.SpawnLength=99999
track:AddKeyFrame(0,99999,0.1)
act_tuowei:AddTrack(track)

act_pose=Action()
act_pose.Length=9999
local track=ActionTrackPose(3)
track.InterpolateX:AddNode(0,0,0,0)
track.InterpolateX:AddNode(3,-5,0,0)
track.InterpolateY:AddNode(0,0,0,0)
track.InterpolateY:AddNode(3,1,0,0)
track:AddKeyFrame(1)
act_pose:AddTrack(track)
