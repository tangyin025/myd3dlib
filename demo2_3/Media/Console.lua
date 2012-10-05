dofile "Loader.lua"
dofile "Font.lua"
dofile "CommonUI.lua"

module("Console", package.seeall)

dlg=Dialog()
dlg.Color=ARGB(197,0,0,0)
-- dlg.Location=Vector2(50,95)
dlg.Size=Vector2(700,410)
dlg.Skin.Font=_Font.font1
dlg.Skin.TextColor=ARGB(255,255,255,255)
dlg.Skin.TextAlign=Font.AlignLeftTop
dlg.EventAlign=function(args)
	CommonUI.UpdateDlgViewProj(dlg,args.vp.x,args.vp.y)
	dlg.Location=Vector2(50,95)
end

local panel=MessagePanel()
panel.Color=ARGB(0,0,0,0)
panel.Location=Vector2(5,5)
panel.Size=Vector2(700-5-5,410-5-5-20)
panel.Skin.Font=_Font.font1
panel.scrollbar.Color=ARGB(15,255,255,255)
panel.scrollbar.Location=Vector2(panel.Size.x - 20, 0)
panel.scrollbar.Size=Vector2(20, panel.Size.y)
panel.scrollbar.nPageSize=3
dlg:InsertControl(panel)

local e_texts={}
local e_texts_idx=0
local edit=ConsoleEditBox()
edit.Color=ARGB(15,255,255,255)
edit.Location=Vector2(5, 410-5-20)
edit.Size=Vector2(700-5-5,20)
edit.Border=Vector4(0,0,0,0)
edit.Text="在这里输入命令"
edit.Skin.Font=_Font.font1
edit.Skin.TextColor=ARGB(255,63,188,239)
edit.EventEnter=function()
	-- 使用临时变量可以减少 wstou8的转换次数
	local code=tostring(edit.Text)
	if string.len(code) > 0 then
		edit.Text=""
		table.insert(e_texts, code)
		if #e_texts > 16 then
			table.remove(e_texts,1)
		end
		e_texts_idx=#e_texts+1
		panel:AddLine(code, edit.Skin.TextColor)
		game:ExecuteCode(code)
	end
end
edit.EventKeyUp=function()
	if e_texts_idx > 1 then
		e_texts_idx=e_texts_idx-1
		edit.Text=e_texts[e_texts_idx]
	end
end
edit.EventKeyDown=function()
	if e_texts_idx < #e_texts then
		e_texts_idx=e_texts_idx+1
		edit.Text=e_texts[e_texts_idx]
	end
end
dlg:InsertControl(edit)

game.console=dlg

game.panel=panel