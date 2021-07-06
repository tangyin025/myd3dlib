skybox_tex_path={
	"texture/cloudy_noon_FR.jpg",
	"texture/cloudy_noon_UP.jpg",
	"texture/cloudy_noon_BK.jpg",
	"texture/cloudy_noon_DN.jpg",
	"texture/cloudy_noon_RT.jpg",
	"texture/cloudy_noon_LF.jpg",
}
for act in theApp.MainWnd.selactors do
	print(os.date("%c"),"add sky box for "..act.Name)
	for i=0,5,1 do
		-- print(i)
		local mesh_cmp=MeshComponent(NamedObject.MakeUniqueName("editor_sky_cmp"))
		mesh_cmp.MeshPath="mesh/Cube6Plane.mesh.xml"
		mesh_cmp.MeshSubMeshId=i
		mesh_cmp.Material=Material()
		mesh_cmp.Material.Shader="shader/mtl_Skybox.fx"
		mesh_cmp.Material:ParseShaderParameters()
		mesh_cmp.Material.CullMode=Material.CullModeCCW
		mesh_cmp.Material.ZEnable=false
		mesh_cmp.Material:SetParameter("g_DiffuseTexture",skybox_tex_path[i+1])
		act:AddComponent(mesh_cmp)
	end
	break
end
