for act in theApp.MainWnd.selactors do
	act:CreateRigidActor(Actor.eRIGID_STATIC)
	for cmp in act.Cmps do
		cmp:CreateTriangleMeshShape(cmp.MeshPath..".pxtrianglemesh_"..cmp.MeshSubMeshName,theApp.MainWnd.CollectionObjs)
		cmp.SimulationFilterWord0=1
		cmp.QueryFilterWord0=1
	end
end
