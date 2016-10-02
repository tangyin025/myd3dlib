require "CommonUI.lua"
module("Settings",package.seeall)

dlg=Dialog()
dlg.Name="Settings"
dlg.Color=ARGB(150,0,0,0)
dlg.Size=Vector2(640,480)
dlg.Skin=CommonUI.com_dlg_skin

local lbl_title=Control()
lbl_title.Location=Vector2(17,13)
lbl_title.Size=Vector2(256,42)
lbl_title.Color=ARGB(255,255,255,255)
lbl_title.Skin=ControlSkin()
lbl_title.Skin.Image=ControlImage()
lbl_title.Skin.Image.Texture=game:LoadTexture("texture/CommonUI.png")
lbl_title.Skin.Image.Rect=Rectangle(0,0,256,42)
lbl_title.Skin.Image.Border=Vector4(0,0,0,0)
dlg:InsertControl(lbl_title)

local btn_ok=Button()
btn_ok.Name="btn_ok"
btn_ok.Location=Vector2(230,439)
btn_ok.Size=Vector2(80,32)
btn_ok.Text="OK"
btn_ok.Skin=CommonUI.com_btn_skin
dlg:InsertControl(btn_ok)

local btn_cancel=Button()
btn_cancel.Name="btn_cancel"
btn_cancel.Location=Vector2(315,439)
btn_cancel.Size=Vector2(80,32)
btn_cancel.Text="Cancel"
btn_cancel.Skin=CommonUI.com_btn_skin
dlg:InsertControl(btn_cancel)

local item_y=390
local item_height=30
local lbl_vertical_sync=Static()
lbl_vertical_sync.Location=Vector2(0,item_y)
lbl_vertical_sync.Size=Vector2(190,22)
lbl_vertical_sync.Text="Vertical Sync"
lbl_vertical_sync.Skin=CommonUI.com_lbl_skin
lbl_vertical_sync.Skin.TextAlign=Font.AlignRightMiddle
dlg:InsertControl(lbl_vertical_sync)

local cbx_vertical_sync=ComboBox()
cbx_vertical_sync.Name="cbx_vertical_sync"
cbx_vertical_sync.Location=Vector2(201,item_y)
cbx_vertical_sync.Size=Vector2(304,22)
cbx_vertical_sync.ScrollbarWidth=20
cbx_vertical_sync.DropdownSize=Vector2(304-20,130)
cbx_vertical_sync.Border=Vector4(20,0,20,0)
cbx_vertical_sync.ItemHeight=22
cbx_vertical_sync.Skin=CommonUI.com_cbx_skin
dlg:InsertControl(cbx_vertical_sync)

item_y=item_y-item_height
local lbl_vertex_processing=Static()
lbl_vertex_processing.Location=Vector2(0,item_y)
lbl_vertex_processing.Size=Vector2(190,22)
lbl_vertex_processing.Text="Vertex Processing"
lbl_vertex_processing.Skin=CommonUI.com_lbl_skin
lbl_vertex_processing.Skin.TextAlign=Font.AlignRightMiddle
dlg:InsertControl(lbl_vertex_processing)

local cbx_vertex_processing=ComboBox()
cbx_vertex_processing.Name="cbx_vertex_processing"
cbx_vertex_processing.Location=Vector2(201,item_y)
cbx_vertex_processing.Size=Vector2(304,22)
cbx_vertex_processing.ScrollbarWidth=20
cbx_vertex_processing.DropdownSize=Vector2(304-20,130)
cbx_vertex_processing.Border=Vector4(20,0,20,0)
cbx_vertex_processing.ItemHeight=22
cbx_vertex_processing.Skin=CommonUI.com_cbx_skin
dlg:InsertControl(cbx_vertex_processing)

item_y=item_y-item_height
local lbl_multisample_quality=Static()
lbl_multisample_quality.Location=Vector2(0,item_y)
lbl_multisample_quality.Size=Vector2(190,22)
lbl_multisample_quality.Text="Multisample Quality"
lbl_multisample_quality.Skin=CommonUI.com_lbl_skin
lbl_multisample_quality.Skin.TextAlign=Font.AlignRightMiddle
dlg:InsertControl(lbl_multisample_quality)

local cbx_multisample_quality=ComboBox()
cbx_multisample_quality.Name="cbx_multisample_quality"
cbx_multisample_quality.Location=Vector2(201,item_y)
cbx_multisample_quality.Size=Vector2(304,22)
cbx_multisample_quality.ScrollbarWidth=20
cbx_multisample_quality.DropdownSize=Vector2(304-20,130)
cbx_multisample_quality.Border=Vector4(20,0,20,0)
cbx_multisample_quality.ItemHeight=22
cbx_multisample_quality.Skin=CommonUI.com_cbx_skin
dlg:InsertControl(cbx_multisample_quality)

item_y=item_y-item_height
local lbl_multisample_type=Static()
lbl_multisample_type.Location=Vector2(0,item_y)
lbl_multisample_type.Size=Vector2(190,22)
lbl_multisample_type.Text="Multisample Type"
lbl_multisample_type.Skin=CommonUI.com_lbl_skin
lbl_multisample_type.Skin.TextAlign=Font.AlignRightMiddle
dlg:InsertControl(lbl_multisample_type)

local cbx_multisample_type=ComboBox()
cbx_multisample_type.Name="cbx_multisample_type"
cbx_multisample_type.Location=Vector2(201,item_y)
cbx_multisample_type.Size=Vector2(304,22)
cbx_multisample_type.ScrollbarWidth=20
cbx_multisample_type.DropdownSize=Vector2(304-20,130)
cbx_multisample_type.Border=Vector4(20,0,20,0)
cbx_multisample_type.ItemHeight=22
cbx_multisample_type.Skin=CommonUI.com_cbx_skin
dlg:InsertControl(cbx_multisample_type)

item_y=item_y-item_height
local lbl_depth_stencil_format=Static()
lbl_depth_stencil_format.Location=Vector2(0,item_y)
lbl_depth_stencil_format.Size=Vector2(190,22)
lbl_depth_stencil_format.Text="Depth/Stencil Format"
lbl_depth_stencil_format.Skin=CommonUI.com_lbl_skin
lbl_depth_stencil_format.Skin.TextAlign=Font.AlignRightMiddle
dlg:InsertControl(lbl_depth_stencil_format)

local cbx_depth_stencil_format=ComboBox()
cbx_depth_stencil_format.Name="cbx_depth_stencil_format"
cbx_depth_stencil_format.Location=Vector2(201,item_y)
cbx_depth_stencil_format.Size=Vector2(304,22)
cbx_depth_stencil_format.ScrollbarWidth=20
cbx_depth_stencil_format.DropdownSize=Vector2(304-20,130)
cbx_depth_stencil_format.Border=Vector4(20,0,20,0)
cbx_depth_stencil_format.ItemHeight=22
cbx_depth_stencil_format.Skin=CommonUI.com_cbx_skin
dlg:InsertControl(cbx_depth_stencil_format)

item_y=item_y-item_height
local lbl_back_buffer_format=Static()
lbl_back_buffer_format.Location=Vector2(0,item_y)
lbl_back_buffer_format.Size=Vector2(190,22)
lbl_back_buffer_format.Text="Back Buffer Format"
lbl_back_buffer_format.Skin=CommonUI.com_lbl_skin
lbl_back_buffer_format.Skin.TextAlign=Font.AlignRightMiddle
dlg:InsertControl(lbl_back_buffer_format)

local cbx_back_buffer_format=ComboBox()
cbx_back_buffer_format.Name="cbx_back_buffer_format"
cbx_back_buffer_format.Location=Vector2(201,item_y)
cbx_back_buffer_format.Size=Vector2(304,22)
cbx_back_buffer_format.ScrollbarWidth=20
cbx_back_buffer_format.DropdownSize=Vector2(304-20,130)
cbx_back_buffer_format.Border=Vector4(20,0,20,0)
cbx_back_buffer_format.ItemHeight=22
cbx_back_buffer_format.Skin=CommonUI.com_cbx_skin
dlg:InsertControl(cbx_back_buffer_format)

item_y=item_y-item_height
local lbl_refresh_rate=Static()
lbl_refresh_rate.Location=Vector2(0,item_y)
lbl_refresh_rate.Size=Vector2(190,22)
lbl_refresh_rate.Text="Refresh Rate"
lbl_refresh_rate.Skin=CommonUI.com_lbl_skin
lbl_refresh_rate.Skin.TextAlign=Font.AlignRightMiddle
dlg:InsertControl(lbl_refresh_rate)

local cbx_refresh_rate=ComboBox()
cbx_refresh_rate.Name="cbx_refresh_rate"
cbx_refresh_rate.Location=Vector2(201,item_y)
cbx_refresh_rate.Size=Vector2(304,22)
cbx_refresh_rate.ScrollbarWidth=20
cbx_refresh_rate.DropdownSize=Vector2(304-20,130)
cbx_refresh_rate.Border=Vector4(20,0,20,0)
cbx_refresh_rate.ItemHeight=22
cbx_refresh_rate.Skin=CommonUI.com_cbx_skin
dlg:InsertControl(cbx_refresh_rate)

item_y=item_y-item_height
local lbl_resolution=Static()
lbl_resolution.Location=Vector2(0,item_y)
lbl_resolution.Size=Vector2(190,22)
lbl_resolution.Text="Resolution"
lbl_resolution.Skin=CommonUI.com_lbl_skin
lbl_resolution.Skin.TextAlign=Font.AlignRightMiddle
dlg:InsertControl(lbl_resolution)

local cbx_resolution=ComboBox()
cbx_resolution.Name="cbx_resolution"
cbx_resolution.Location=Vector2(201,item_y)
cbx_resolution.Size=Vector2(304,22)
cbx_resolution.ScrollbarWidth=20
cbx_resolution.DropdownSize=Vector2(304-20,130)
cbx_resolution.Border=Vector4(20,0,20,0)
cbx_resolution.ItemHeight=22
cbx_resolution.Skin=CommonUI.com_cbx_skin
dlg:InsertControl(cbx_resolution)

item_y=item_y-item_height
local lbl_adapter_format=Static()
lbl_adapter_format.Location=Vector2(0,item_y)
lbl_adapter_format.Size=Vector2(190,22)
lbl_adapter_format.Text="Adapter Format"
lbl_adapter_format.Skin=CommonUI.com_lbl_skin
lbl_adapter_format.Skin.TextAlign=Font.AlignRightMiddle
dlg:InsertControl(lbl_adapter_format)

local cbx_adapter_format=ComboBox()
cbx_adapter_format.Name="cbx_adapter_format"
cbx_adapter_format.Location=Vector2(201,item_y)
cbx_adapter_format.Size=Vector2(304,22)
cbx_adapter_format.ScrollbarWidth=20
cbx_adapter_format.DropdownSize=Vector2(304-20,130)
cbx_adapter_format.Border=Vector4(20,0,20,0)
cbx_adapter_format.ItemHeight=22
cbx_adapter_format.Skin=CommonUI.com_cbx_skin
dlg:InsertControl(cbx_adapter_format)

item_y=item_y-item_height
local chx_windowed=CheckBox()
chx_windowed.Name="chx_windowed"
chx_windowed.Location=Vector2(239,item_y)
chx_windowed.Size=Vector2(120,22)
chx_windowed.Text="Windowed"
chx_windowed.Skin=CommonUI.com_chx_skin
dlg:InsertControl(chx_windowed)

local chx_full_screen=CheckBox()
chx_full_screen.Name="chx_full_screen"
chx_full_screen.Location=Vector2(357,item_y)
chx_full_screen.Size=Vector2(120,22)
chx_full_screen.Text="Full Screen"
chx_full_screen.Skin=CommonUI.com_chx_skin
dlg:InsertControl(chx_full_screen)

item_y=item_y-item_height
local lbl_render_device=Static()
lbl_render_device.Location=Vector2(0,item_y)
lbl_render_device.Size=Vector2(190,22)
lbl_render_device.Text="Render Device"
lbl_render_device.Skin=CommonUI.com_lbl_skin
lbl_render_device.Skin.TextAlign=Font.AlignRightMiddle
dlg:InsertControl(lbl_render_device)

local cbx_render_device=ComboBox()
cbx_render_device.Name="cbx_render_device"
cbx_render_device.Location=Vector2(201,item_y)
cbx_render_device.Size=Vector2(304,22)
cbx_render_device.ScrollbarWidth=20
cbx_render_device.DropdownSize=Vector2(304-20,130)
cbx_render_device.Border=Vector4(20,0,20,0)
cbx_render_device.ItemHeight=22
cbx_render_device.Skin=CommonUI.com_cbx_skin
dlg:InsertControl(cbx_render_device)

item_y=item_y-item_height
local lbl_display_adapter=Static()
lbl_display_adapter.Location=Vector2(0,item_y)
lbl_display_adapter.Size=Vector2(190,22)
lbl_display_adapter.Text="Display Adapter"
lbl_display_adapter.Skin=CommonUI.com_lbl_skin
lbl_display_adapter.Skin.TextAlign=Font.AlignRightMiddle
dlg:InsertControl(lbl_display_adapter)

local cbx_display_adapter=ComboBox()
cbx_display_adapter.Name="cbx_display_adapter"
cbx_display_adapter.Location=Vector2(201,item_y)
cbx_display_adapter.Size=Vector2(304,22)
cbx_display_adapter.ScrollbarWidth=20
cbx_display_adapter.DropdownSize=Vector2(304-20,130)
cbx_display_adapter.Border=Vector4(20,0,20,0)
cbx_display_adapter.ItemHeight=22
cbx_display_adapter.Skin=CommonUI.com_cbx_skin
dlg:InsertControl(cbx_display_adapter)
