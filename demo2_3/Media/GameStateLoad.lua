
t=game:AddTimer(0, function(args)
	game:RemoveTimer(t)
	t=game:AddTimer(0, function(args)
		game:process_event(GameEventLoadOver())
		game:RemoveTimer(t)
	end)
end)