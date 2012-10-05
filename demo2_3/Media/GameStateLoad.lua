
t=Timer(0.1)
t.EventTimer=function()
	game:ChangeState("GameStateMain")
	game:RemoveTimer(t)
end
game:InsertTimer(t)