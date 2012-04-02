#include "Console.h"
#include "Game.h"

MessagePanel::MessagePanel(void)
	: m_lbegin(0)
	, m_lend(0)
{
}

MessagePanel::~MessagePanel(void)
{
}

void MessagePanel::OnRender(IDirect3DDevice9 * pd3dDevice, float fElapsedTime, const my::Vector2 & Offset)
{
	my::Control::OnRender(pd3dDevice, fElapsedTime, Offset);

	if(m_Skin && m_Skin->m_Font)
	{
		my::Rectangle Rect(my::Rectangle::LeftTop(m_Location + Offset, m_Size));

		my::ScrollBarPtr scrollbar = m_scrollbar.lock();
		_ASSERT(scrollbar);

		int i = MoveLineIndex(m_lbegin, scrollbar->m_nPosition);
		float y = Rect.t;
		for(; i != m_lend && y <= Rect.b - m_Skin->m_Font->m_LineHeight; i = MoveLineIndex(i, 1), y += m_Skin->m_Font->m_LineHeight)
		{
			m_Skin->m_Font->DrawString(m_lines[i].m_Text.c_str(),
				my::Rectangle(Rect.l, y, Rect.r, y + m_Skin->m_Font->m_LineHeight), m_lines[i].m_Color);
		}
	}
}

int MessagePanel::MoveLineIndex(int index, int step)
{
	return (index + step) % LineArray::size();
}

int MessagePanel::LineIndexDistance(int start, int end)
{
	if(end >= start)
		return end - start;

	return LineArray::size() - start + end;
}

void MessagePanel::AddLine(LPCWSTR pString, D3DCOLOR Color)
{
	if(*pString && m_Skin && m_Skin->m_Font)
	{
		int nCP = m_Skin->m_Font->XtoCP(pString, m_Size.x);

		m_lines[m_lend].m_Text.resize(nCP);
		memcpy(&m_lines[m_lend].m_Text[0], pString, nCP * sizeof(wchar_t));
		m_lines[m_lend].m_Color = Color;

		m_lend = MoveLineIndex(m_lend, 1);
		if(m_lend == m_lbegin)
		{
			m_lbegin = MoveLineIndex(m_lbegin, 1);
		}

		my::ScrollBarPtr scrollbar = m_scrollbar.lock();
		_ASSERT(scrollbar);
		scrollbar->m_nStart = 0;
		scrollbar->m_nEnd = LineIndexDistance(m_lbegin, m_lend);
		scrollbar->m_nPageSize = (int)(m_Size.y / m_Skin->m_Font->m_LineHeight);
		scrollbar->m_nPosition = scrollbar->m_nEnd - scrollbar->m_nPageSize;

		AddLine(pString + nCP, Color);
	}
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

	m_edit->this_ptr = m_edit;
	m_edit->EventEnter = fastdelegate::MakeDelegate(this, &Console::OnExecute);

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
	m_panel->m_scrollbar = m_scrollbar;
	m_panel->m_Color = D3DCOLOR_ARGB(35,0,0,255);
	m_panel->m_Location = my::Vector2(m_Border.x, m_Border.y);
	m_panel->m_Size = panel_size;
	m_panel->m_Skin = m_Skin;
	m_Controls.insert(m_panel);
}

Console::~Console(void)
{
}

void Console::OnExecute(my::ControlPtr ctrl)
{
	my::EditBoxPtr edit = boost::dynamic_pointer_cast<my::EditBox, my::Control>(ctrl);
	_ASSERT(edit);

	m_panel->AddLine(edit->m_Text.c_str(), D3DCOLOR_ARGB(255,63,188,239));

	edit->m_Text.clear();
	edit->m_nCaret = 0;
	edit->m_nFirstVisible = 0;
}
