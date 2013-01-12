
t=game:AddTimer(0.1, function(args)
	game:ChangeState("GameStateMain")
	game:RemoveTimer(t)
end)