dofile "CommonUI.lua"

module("LoadingUI", package.seeall)

dlg=Dialog()
dlg.Color=ARGB(150,0,0,0)
dlg.Size=Vector2(500,100)
dlg.Skin=CommonUI.com_lbl_skin
dlg.EventAlign=function(args)
	dlg.Location=(game.DlgViewport-dlg.Size)*0.5
end

pgs = ProgressBar()
pgs.Size=Vector2(400,50)
pgs.Location=(dlg.Size-pgs.Size)*0.5
pgs.Skin=CommonUI.com_pgs_skin
dlg:InsertControl(pgs)

game:InsertDlg(dlg)