local act=Actor("sky",Vector3(0),Quaternion.Identity(),Vector3(1000),AABB(-1,1))
theApp.MainWnd:AddEntity(act)
theApp.MainWnd:PushToActorList(act)

skybox_tex_path={
	"texture/cloudy_noon_FR.jpg",
	"texture/cloudy_noon_BK.jpg",
	"texture/cloudy_noon_LF.jpg",
	"texture/cloudy_noon_RT.jpg",
	"texture/cloudy_noon_UP.jpg",
	"texture/cloudy_noon_DN.jpg",
}
print(os.date("%c"),"add sky box for "..act.Name)
for i=1,6 do
	-- print(i)
	local mesh_cmp=MeshComponent(act.Name.."_cmp"..i-1)
	mesh_cmp.MeshPath="mesh/Cube6Plane.mesh.xml"
	mesh_cmp.MeshSubMeshId=i-1
	mesh_cmp.Material=Material()
	mesh_cmp.Material.Shader="shader/mtl_simpletexture2.fx"
	mesh_cmp.Material:ParseShaderParameters()
	mesh_cmp.Material.CullMode=Material.D3DCULL_CCW
	mesh_cmp.Material.ZEnable=false
	mesh_cmp.Material:SetParameter("g_DiffuseTexture",skybox_tex_path[i])
	act:InsertComponent(mesh_cmp)
end
