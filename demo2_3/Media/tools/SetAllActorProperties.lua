for act in theApp.MainWnd.allactors do
	act.CullingDist=500
	for cmp in act.Cmps do
		if cmp.Type==Component.ComponentTypeMesh then
			cmp.bInstance=true
		end
	end
end
