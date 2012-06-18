#include "Console.h"
#include "Game.h"

MessagePanel::MessagePanel(void)
	: m_lbegin(0)
	, m_lend(0)
{
	m_scrollbar = my::ScrollBarPtr(new my::ScrollBar());
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

		int i = MoveLineIndex(m_lbegin, m_scrollbar->m_nPosition);
		float y = Rect.t;
		for(; i != m_lend && y <= Rect.b - m_Skin->m_Font->m_LineHeight; i = MoveLineIndex(i, 1), y += m_Skin->m_Font->m_LineHeight)
		{
			m_Skin->m_Font->DrawString(m_lines[i].m_Text.c_str(),
				my::Rectangle(Rect.l, y, Rect.r, y + m_Skin->m_Font->m_LineHeight), m_lines[i].m_Color);
		}
	}

	m_scrollbar->OnRender(pd3dDevice, fElapsedTime, m_Location + Offset);
}

bool MessagePanel::MsgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if(m_scrollbar->MsgProc(hWnd, uMsg, wParam, lParam))
		return true;

	return Control::MsgProc(hWnd, uMsg, wParam, lParam);
}

bool MessagePanel::HandleKeyboard(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if(m_scrollbar->HandleKeyboard(uMsg, wParam, lParam))
		return true;

	return Control::HandleKeyboard(uMsg, wParam, lParam);
}

bool MessagePanel::HandleMouse(UINT uMsg, const my::Vector2 & pt, WPARAM wParam, LPARAM lParam)
{
	if(m_scrollbar->HandleMouse(uMsg, pt, wParam, lParam))
		return true;

	return Control::HandleMouse(uMsg, pt, wParam, lParam);
}

int MessagePanel::MoveLineIndex(int index, int step)
{
	return (index + step) % _countof(m_lines);
}

bool MessagePanel::CanHaveFocus(void)
{
	return m_bVisible && m_bEnabled;
}

int MessagePanel::LineIndexDistance(int start, int end)
{
	if(end >= start)
		return end - start;

	return _countof(m_lines) - start + end;
}

void MessagePanel::_update_scrollbar(void)
{
	_ASSERT(m_Skin && m_Skin->m_Font);
	m_scrollbar->m_nStart = 0;
	m_scrollbar->m_nEnd = LineIndexDistance(m_lbegin, m_lend);
	m_scrollbar->m_nPageSize = (int)(m_Size.y / m_Skin->m_Font->m_LineHeight);
	m_scrollbar->m_nPosition = m_scrollbar->m_nEnd - m_scrollbar->m_nPageSize;
}

void MessagePanel::_push_enter(void)
{
	m_lend = MoveLineIndex(m_lend, 1);
	if(m_lend == m_lbegin)
	{
		m_lbegin = MoveLineIndex(m_lbegin, 1);
	}
	m_lines[m_lend].m_Text.clear();
	_update_scrollbar();
}

void MessagePanel::_push_sline(LPCWSTR pString, D3DCOLOR Color)
{
	// _push_sline 只是在 lend 处插入字符串，但由于 lend 位于 message panel 底部之后，
	// 所以如果没有 _push_enter，lend 是不会显示出来的
	_ASSERT(m_Skin && m_Skin->m_Font);
	std::wstring & lend_str = m_lines[m_lend].m_Text;
	float lend_x = m_Skin->m_Font->CPtoX(lend_str.c_str(), lend_str.length());
	float remain_x = m_Size.x - m_scrollbar->m_Size.x - lend_x;
	int nCP = m_Skin->m_Font->XtoCP(pString, remain_x);
	//_ASSERT(wcslen(pString) == 0 || nCP > 0);
	if(lend_str.empty() && nCP <= 0)
		return;

	lend_str.insert(lend_str.length(), pString, nCP);
	m_lines[m_lend].m_Color = Color;

	if(pString[nCP])
	{
		_push_enter();
		_push_sline(&pString[nCP], Color);
	}
}

void MessagePanel::AddLine(const std::wstring & str, D3DCOLOR Color)
{
	if(m_Skin && m_Skin->m_Font)
	{
		_push_sline(str.c_str(), Color);
		_push_enter();
	}
}

void MessagePanel::puts(const std::wstring & str, D3DCOLOR Color)
{
	std::wstring::size_type lpos = 0;
	for(; lpos < str.length(); )
	{
		std::wstring::size_type rpos = str.find(L'\n', lpos);
		if(std::wstring::npos == rpos)
		{
			_push_sline(str.substr(lpos).c_str(), Color);
			break;
		}
		_push_sline(str.substr(lpos, rpos - lpos).c_str(), Color);
		_push_enter();
		lpos = rpos + 1;
	}
}

ConsoleEditBox::ConsoleEditBox(void)
{
}

ConsoleEditBox::~ConsoleEditBox(void)
{
}

bool ConsoleEditBox::HandleKeyboard(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if(m_bEnabled && m_bVisible)
	{
		switch(uMsg)
		{
		case WM_KEYDOWN:
			switch(wParam)
			{
			case VK_UP:
				if(EventPrevLine)
					EventPrevLine(my::EventArgsPtr(new my::EventArgs));
				ResetCaretBlink();
				return true;

			case VK_DOWN:
				if(EventNextLine)
					EventNextLine(my::EventArgsPtr(new my::EventArgs));
				ResetCaretBlink();
				return true;
			}
			break;
		}
	}
	return ImeEditBox::HandleKeyboard(uMsg, wParam, lParam);
}
//
//Console::Console(void)
//{
//	m_Color = D3DCOLOR_ARGB(197,0,0,0);
//	m_Location = my::Vector2(50,95);
//	m_Size = my::Vector2(700,410);
//	m_Border = my::Vector4(5,5,5,5);
//
//	m_Skin = my::ControlSkinPtr(new my::ControlSkin());
//	m_Skin->m_Font = Game::getSingleton().m_uiFnt;
//	m_Skin->m_TextColor = D3DCOLOR_ARGB(255,255,255,255);
//	m_Skin->m_TextAlign = my::Font::AlignLeftTop;
//
//	const my::Vector2 edit_size(m_Size.x - m_Border.x - m_Border.z, 20);
//	m_edit = my::ImeEditBoxPtr(new my::ImeEditBox());
//	m_edit->m_Color = D3DCOLOR_ARGB(15,255,255,255);
//	m_edit->m_Location = my::Vector2(m_Border.x, m_Size.y - m_Border.w - edit_size.y);
//	m_edit->m_Size = edit_size;
//	m_edit->m_Border = my::Vector4(0,0,0,0);
//	m_edit->m_Text = L"在这里输入命令";
//	m_edit->m_Skin = my::EditBoxSkinPtr(new my::EditBoxSkin());
//	m_edit->m_Skin->m_Font = m_Skin->m_Font;
//	m_edit->m_Skin->m_TextColor = D3DCOLOR_ARGB(255,63,188,239);
//	InsertControl(m_edit);
//
//	m_panel = MessagePanelPtr(new MessagePanel());
//	m_panel->m_Color = D3DCOLOR_ARGB(0,0,0,0);
//	m_panel->m_Location = my::Vector2(m_Border.x, m_Border.y);
//	m_panel->m_Size = my::Vector2(m_Size.x - m_Border.x - m_Border.z, m_Size.y - m_Border.y - m_edit->m_Size.y - m_Border.w);
//	m_panel->m_Skin = m_Skin;
//	m_panel->m_scrollbar->m_Color = D3DCOLOR_ARGB(15,255,255,255);
//	m_panel->m_scrollbar->m_Location = my::Vector2(m_panel->m_Size.x - 20, 0);
//	m_panel->m_scrollbar->m_Size = my::Vector2(20, m_panel->m_Size.y);
//	m_panel->m_scrollbar->m_nPageSize = 3;
//	m_panel->m_scrollbar->m_Skin = my::ScrollBarSkinPtr(new my::ScrollBarSkin());
//	InsertControl(m_panel);
//}
//
//Console::~Console(void)
//{
//}
