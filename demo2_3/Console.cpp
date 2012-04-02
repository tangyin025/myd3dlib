#include "Console.h"
#include "Game.h"

MessagePanel::MessagePanel(void)
{
}

MessagePanel::~MessagePanel(void)
{
}

void MessagePanel::OnRender(IDirect3DDevice9 * pd3dDevice, float fElapsedTime, const my::Vector2 & Offset)
{
	my::Control::OnRender(pd3dDevice, fElapsedTime, Offset);
}

Console::Console(void)
{
	m_Color = D3DCOLOR_ARGB(197,0,0,0);
	m_Location = my::Vector2(50,100);
	m_Size = my::Vector2(700,400);
	m_Border = my::Vector4(5,5,5,5);

	Game * game = dynamic_cast<Game *>(my::DxutApp::getSingletonPtr());
	_ASSERT(game);

	m_Skin = game->m_defDlgSkin;

	const my::Vector2 edit_size(m_Size.x - m_Border.x - m_Border.z, 20);
	m_edit = my::ImeEditBoxPtr(new my::ImeEditBox());
	m_edit->m_Color = D3DCOLOR_ARGB(35,0,255,0);
	m_edit->m_Location = my::Vector2(m_Border.x, m_Size.y - m_Border.w - edit_size.y);
	m_edit->m_Size = edit_size;
	m_edit->m_Border = my::Vector4(0,0,0,0);
	m_edit->m_Text = L"在这里输入命令";
	m_edit->m_Skin = my::EditBoxSkinPtr(new my::EditBoxSkin());
	m_edit->m_Skin->m_Font = m_Skin->m_Font;
	m_edit->m_Skin->m_TextColor = D3DCOLOR_ARGB(255,255,255,255);
	m_Controls.insert(m_edit);

	const my::Vector2 scroll_size(20, m_Size.y - m_Border.y - m_edit->m_Size.y - m_Border.w);
	m_scrollbar = my::ScrollBarPtr(new my::ScrollBar());
	m_scrollbar->m_Color = D3DCOLOR_ARGB(35,255,0,0);
	m_scrollbar->m_Location = my::Vector2(m_Size.x - m_Border.z - scroll_size.x, m_Border.y);
	m_scrollbar->m_Size = scroll_size;
	m_scrollbar->m_nPageSize = 3;
	m_scrollbar->m_Skin = my::ScrollBarSkinPtr(new my::ScrollBarSkin());
	m_Controls.insert(m_scrollbar);

	const my::Vector2 panel_size(
		m_Size.x - m_Border.x - m_scrollbar->m_Size.x - m_Border.z, m_Size.y - m_Border.y - m_edit->m_Size.y - m_Border.w);
	m_panel = MessagePanelPtr(new MessagePanel());
	m_panel->m_Color = D3DCOLOR_ARGB(35,0,0,255);
	m_panel->m_Location = my::Vector2(m_Border.x, m_Border.y);
	m_panel->m_Size = panel_size;
	m_panel->m_Skin = m_Skin;
	m_Controls.insert(m_panel);
}

Console::~Console(void)
{
}
