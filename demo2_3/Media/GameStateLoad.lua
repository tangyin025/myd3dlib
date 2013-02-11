
t=game:AddTimer(0, function(args)
	game:RemoveTimer(t)
	t=game:AddTimer(0, function(args)
		game:ChangeState("GameStateMain")
		game:RemoveTimer(t)
	end)
end)