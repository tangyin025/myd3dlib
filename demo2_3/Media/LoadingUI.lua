dofile "CommonUI.lua"
module("LoadingUI", package.seeall)

dlg=Dialog()
dlg.Name="LoadingUI"
dlg.Color=ARGB(150,0,0,0)
dlg.Size=Vector2(500,100)
dlg.Skin=CommonUI.com_lbl_skin

local pgs=ProgressBar()
pgs.Name="pgs"
pgs.Size=Vector2(400,50)
pgs.Location=(dlg.Size-pgs.Size)*0.5
pgs.Skin=CommonUI.com_pgs_skin
dlg:InsertControl(pgs)
