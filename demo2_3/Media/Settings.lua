module("Settings",package.seeall)

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

local function SettingsButton(x,y,text)
	local btn=Button()
	btn.Location=Vector2(x,y)
	btn.Size=Vector2(80,32)
	btn.Text=text
	btn.Skin=CommonUI.com_btn_skin
	return btn
end

local function SettingsCheckBox(x,y,text)
	local chx=CheckBox()
	chx.Location=Vector2(x,y)
	chx.Size=Vector2(120,22)
	chx.Text=text
	chx.Skin=CommonUI.com_chx_skin
	return chx
end

local function SettingsLabel(y,text)
	local lbl=Static()
	lbl.Location=Vector2(0,y)
	lbl.Size=Vector2(190,22)
	lbl.Text=text
	lbl.Skin=CommonUI.com_lbl_skin
	lbl.Skin.TextAlign=Font.AlignRightMiddle
	return lbl
end

local function SettingsComboBox(y)
	local cbx=ComboBox()
	cbx.Location=Vector2(201,y)
	cbx.Size=Vector2(304,22)
	cbx.ScrollbarWidth=20
	cbx.DropdownSize=Vector2(304-20,130)
	cbx.Border=Vector4(20,0,0,0)
	cbx.ItemHeight=22
	cbx.Skin=CommonUI.com_cbx_skin
	return cbx
end

local btn_ok=SettingsButton(230,439,"OK")
btn_ok.EventClick=function(args)
	print("SettingsDlg OK")
end
dlg:InsertControl(btn_ok)

local btn_cancel=SettingsButton(315,439,"Cancel")
btn_cancel.EventClick=function(args)
	print("SettingsDlg Cancel")
end
dlg:InsertControl(btn_cancel)

local item_y=390
local item_height=30
local lbl_vertical_sync=SettingsLabel(item_y,"Vertical Sync")
dlg:InsertControl(lbl_vertical_sync)
local cbx_vertical_sync=SettingsComboBox(item_y)
dlg:InsertControl(cbx_vertical_sync)

item_y=item_y-item_height
local lbl_vertex_processing=SettingsLabel(item_y,"Vertex Processing")
dlg:InsertControl(lbl_vertex_processing)
local cbx_vertex_processing=SettingsComboBox(item_y)
dlg:InsertControl(cbx_vertex_processing)

item_y=item_y-item_height
local lbl_multisample_quality=SettingsLabel(item_y,"Multisample Quality")
dlg:InsertControl(lbl_multisample_quality)
local cbx_multisample_quality=SettingsComboBox(item_y)
dlg:InsertControl(cbx_multisample_quality)

item_y=item_y-item_height
local lbl_multisample_type=SettingsLabel(item_y,"Multisample Type")
dlg:InsertControl(lbl_multisample_type)
local cbx_multisample_type=SettingsComboBox(item_y)
dlg:InsertControl(cbx_multisample_type)

item_y=item_y-item_height
local lbl_depth_format=SettingsLabel(item_y,"Depth/Stencil Format")
dlg:InsertControl(lbl_depth_format)
local cbx_depth_format=SettingsComboBox(item_y)
dlg:InsertControl(cbx_depth_format)

item_y=item_y-item_height
local lbl_back_buffer_format=SettingsLabel(item_y,"Back Buffer Format")
dlg:InsertControl(lbl_back_buffer_format)
local cbx_back_buffer_format=SettingsComboBox(item_y)
dlg:InsertControl(cbx_back_buffer_format)

item_y=item_y-item_height
local lbl_refresh_rate=SettingsLabel(item_y,"Refresh Rate")
dlg:InsertControl(lbl_refresh_rate)
local cbx_refresh_rate=SettingsComboBox(item_y)
dlg:InsertControl(cbx_refresh_rate)

item_y=item_y-item_height
local lbl_resolution=SettingsLabel(item_y,"Resolution")
dlg:InsertControl(lbl_resolution)
local cbx_resolution=SettingsComboBox(item_y)
dlg:InsertControl(cbx_resolution)

item_y=item_y-item_height
local lbl_adapter_format=SettingsLabel(item_y,"Adapter Format")
dlg:InsertControl(lbl_adapter_format)
local cbx_adapter_format=SettingsComboBox(item_y)
dlg:InsertControl(cbx_adapter_format)

item_y=item_y-item_height
local chx_windowed=SettingsCheckBox(239,item_y,"Windowed")
dlg:InsertControl(chx_windowed)
local chx_full_screen=SettingsCheckBox(357,item_y,"Full Screen")
dlg:InsertControl(chx_full_screen)

item_y=item_y-item_height
local lbl_render_device=SettingsLabel(item_y,"Render Device")
dlg:InsertControl(lbl_render_device)
local cbx_render_device=SettingsComboBox(item_y)
cbx_render_device.EventSelectionChanged=function(args)
	OnDeviceTypeChanged()
end
dlg:InsertControl(cbx_render_device)

item_y=item_y-item_height
local lbl_display_adapter=SettingsLabel(item_y,"Display Adapter")
dlg:InsertControl(lbl_display_adapter)
local cbx_display_adapter=SettingsComboBox(item_y)
cbx_display_adapter.EventSelectionChanged=function(args)
	OnAdapterChanged()
end
dlg:InsertControl(cbx_display_adapter)

local local_device_settings=nil

function RefreshDisplayAdapter()
	local_device_settings=game:GetD3D9DeviceSettings()
	local adapter_info_list=game:GetAdapterInfoList()
	cbx_display_adapter:RemoveAllItems()
	for i=0,adapter_info_list:GetSize()-1 do
		local adapter_info=adapter_info_list:GetAt(i)
		cbx_display_adapter:AddItem(adapter_info.szUniqueDescription)
		cbx_display_adapter:SetItemData(i,adapter_info.AdapterOrdinal)
		if local_device_settings.AdapterOrdinal == adapter_info.AdapterOrdinal then
			cbx_display_adapter.Selected=i
		end
	end
	OnAdapterChanged()
end

function OnAdapterChanged()
	if cbx_display_adapter.Selected >= 0 then
		print("Settings::OnAdapterChanged()")
		assert(local_device_settings)
		local adapter_info=game:GetAdapterInfo(local_device_settings.AdapterOrdinal)
		cbx_render_device:RemoveAllItems()
		for i=0,adapter_info.deviceInfoList:GetSize()-1 do
			local device_info=adapter_info.deviceInfoList:GetAt(i)
			cbx_render_device:AddItem(DxutApp.DXUTD3DDeviceTypeToString(device_info.DeviceType))
			cbx_render_device:SetItemData(i,device_info.DeviceType)
			if local_device_settings.DeviceType == device_info.DeviceType then
				cbx_render_device.Selected=i
			end
		end
		OnDeviceTypeChanged()
	end
end

function OnDeviceTypeChanged()
	if cbx_render_device.Selected >= 0 then
		print("Settings::OnDeviceTypeChanged()")
		assert(local_device_settings)
	end
end

dlg.EventRefresh=function(args)
	RefreshDisplayAdapter()
end