module("Settings", package.seeall)

dlg=Dialog()
dlg.Visible=false
dlg.Color=ARGB(150,0,0,0)
dlg.Size=Vector2(640,480)
dlg.Skin.Font=_Font.font1
dlg.Skin.TextColor=ARGB(255,255,255,255)
dlg.Skin.TextAlign=Font.AlignLeftTop
dlg.EventAlign=function(args)
	dlg.Location=Vector2((args.vp.x-dlg.Size.x)*0.5,(args.vp.y-dlg.Size.y)*0.5)
end

game:InsertDlg(dlg)

Hud.btn_change_device.EventClick=function(args)
	dlg.Visible=not dlg.Visible
end