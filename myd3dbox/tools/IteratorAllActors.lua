for act in theApp.MainWnd.allactors do
	for cmp in act.Cmps do
		local mat=cmp.Material
		if mat and string.find(mat.Shader,"mtl_BlinnPhong.fx") then
			print(mat.Shader)
			mat:ReleaseResource()
			mat:ParseShaderParameters()
			mat:RequestResource()
		end
	end
end