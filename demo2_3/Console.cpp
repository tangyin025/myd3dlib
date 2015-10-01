#include "stdafx.h"
#include "Console.h"
#include "Game.h"

using namespace my;

MessagePanel::MessagePanel(void)
	: m_lbegin(0)
	, m_lend(0)
{
	m_scrollbar.reset(new ScrollBar());
	m_scrollbar->m_Parent = this;
}

MessagePanel::~MessagePanel(void)
{
}

void MessagePanel::Draw(UIRender * ui_render, float fElapsedTime, const Vector2 & Offset)
{
	Control::Draw(ui_render, fElapsedTime, Offset);

	if(m_Skin && m_Skin->m_Font)
	{
		my::Rectangle Rect(Rectangle::LeftTop(m_Location + Offset, m_Size));

		int i = MoveLineIndex(m_lbegin, m_scrollbar->m_nPosition);
		float y = Rect.t;
		for(; i != m_lend && y <= Rect.b - m_Skin->m_Font->m_LineHeight; i = MoveLineIndex(i, 1), y += m_Skin->m_Font->m_LineHeight)
		{
			m_Skin->m_Font->PushStringVertices(ui_render, m_lines[i].m_Text.c_str(),
				my::Rectangle(Rect.l, y, Rect.r, y + m_Skin->m_Font->m_LineHeight), m_lines[i].m_Color);
		}
	}

	m_scrollbar->Draw(ui_render, fElapsedTime, m_Location + Offset);
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

bool MessagePanel::HandleMouse(UINT uMsg, const Vector2 & pt, WPARAM wParam, LPARAM lParam)
{
	if(m_scrollbar->HandleMouse(uMsg, pt, wParam, lParam))
		return true;

	return Control::HandleMouse(uMsg, pt, wParam, lParam);
}

bool MessagePanel::CanHaveFocus(void)
{
	return m_bVisible && m_bEnabled;
}

int MessagePanel::MoveLineIndex(int index, int step)
{
	return (index + step) % _countof(m_lines);
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

void MessagePanel::_push_enter(D3DCOLOR Color)
{
	m_lines[m_lend].m_Text.clear();
	m_lines[m_lend].m_Color = Color;
	m_lend = MoveLineIndex(m_lend, 1);
	if(m_lend == m_lbegin)
	{
		m_lbegin = MoveLineIndex(m_lbegin, 1);
	}
	_update_scrollbar();
}

void MessagePanel::_push_line(const std::wstring & str, D3DCOLOR Color)
{
	_ASSERT(m_Skin && m_Skin->m_Font);

	if(!str.empty())
	{
		int last_line_idx = MoveLineIndex(m_lend, -1);
		std::wstring & last_line_str = m_lines[last_line_idx].m_Text;
		float lend_x = m_Skin->m_Font->CPtoX(last_line_str.c_str(), last_line_str.length());
		float remain_x = m_Size.x - m_scrollbar->m_Size.x - lend_x;
		int nCP = m_Skin->m_Font->XtoCP(str.c_str(), remain_x);
		last_line_str.insert(last_line_str.length(), str.c_str(), nCP);

		if(nCP < (int)str.length())
		{
			_push_enter(Color);
			_push_line(str.substr(nCP), Color);
		}
	}
}

void MessagePanel::AddLine(const std::wstring & str, D3DCOLOR Color)
{
	if(m_Skin && m_Skin->m_Font)
	{
		_push_enter(Color);
		puts(str.c_str());
	}
}

void MessagePanel::puts(const std::wstring & str)
{
	if(m_Skin && m_Skin->m_Font)
	{
		D3DCOLOR Color = m_lines[MoveLineIndex(m_lend, -1)].m_Color;
		std::wstring::size_type lpos = 0;
		for(; lpos < str.length(); )
		{
			std::wstring::size_type rpos = str.find(L'\n', lpos);
			if(std::wstring::npos == rpos)
			{
				_push_line(str.substr(lpos).c_str(), Color);
				break;
			}
			_push_line(str.substr(lpos, rpos - lpos).c_str(), Color);
			_push_enter(Color);
			lpos = rpos + 1;
		}
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
				if(EventKeyUp)
					EventKeyUp(&EventArgs());
				ResetCaretBlink();
				return true;

			case VK_DOWN:
				if(EventKeyDown)
					EventKeyDown(&EventArgs());
				ResetCaretBlink();
				return true;
			}
			break;
		}
	}
	return ImeEditBox::HandleKeyboard(uMsg, wParam, lParam);
}

Console::Console(void)
{
	m_Color = D3DCOLOR_ARGB(197,0,0,0);
	m_Size = Vector2(700,410);
	m_Skin->m_Font = Game::getSingleton().m_Font;
	m_Skin->m_TextColor = D3DCOLOR_ARGB(255,255,255,255);
	m_Skin->m_TextAlign = Font::AlignLeftTop;
	EventAlign = boost::bind(&Console::OnEventAlign, this, _1);

	const Vector4 Border(5,5,5,5);

	m_Edit.reset(new ConsoleEditBox());
	m_Edit->m_Color = D3DCOLOR_ARGB(15,255,255,255);
	m_Edit->m_Size = Vector2(m_Size.x - Border.x - Border.z, 20);
	m_Edit->m_Location = Vector2(Border.x, m_Size.y - Border.w - m_Edit->m_Size.y);
	m_Edit->m_Border = Vector4(0,0,0,0);
	m_Edit->m_Skin->m_Font = Game::getSingleton().m_Font;
	m_Edit->m_Skin->m_TextColor = D3DCOLOR_ARGB(255,63,188,239);
	m_Edit->m_Skin->m_TextAlign = Font::AlignLeftMiddle;
	boost::dynamic_pointer_cast<EditBoxSkin>(m_Edit->m_Skin)->m_SelBkColor = D3DCOLOR_ARGB(255,255,128,0);
	m_Edit->EventEnter = boost::bind(&Console::OnEventEnter, this, _1);
	m_Edit->EventKeyUp = boost::bind(&Console::OnEventKeyUp, this, _1);
	m_Edit->EventKeyDown = boost::bind(&Console::OnEventKeyDown, this, _1);
	InsertControl(m_Edit);

	m_Panel.reset(new MessagePanel());
	m_Panel->m_Color = D3DCOLOR_ARGB(0,0,0,0);
	m_Panel->m_Location = Vector2(Border.x, Border.y);
	m_Panel->m_Size = Vector2(m_Size.x - Border.x - Border.z, m_Size.y - Border.y - Border.w - m_Edit->m_Size.y);
	m_Panel->m_Skin->m_Font = Game::getSingleton().m_Font;
	m_Panel->m_scrollbar->m_Color = D3DCOLOR_ARGB(15,255,255,255);
	m_Panel->m_scrollbar->m_Size = Vector2(20, m_Panel->m_Size.y);
	m_Panel->m_scrollbar->m_Location = Vector2(m_Panel->m_Size.x - m_Panel->m_scrollbar->m_Size.x, 0);
	m_Panel->m_scrollbar->m_nPageSize = 3;
	InsertControl(m_Panel);

	m_strIter = m_strList.end();
}

Console::~Console(void)
{
}

void Console::OnEventAlign(EventArgs * args)
{
	m_Location = Vector2(50,95);
}

void Console::OnEventEnter(EventArgs * args)
{
	std::wstring code = m_Edit->m_Text;
	if(!code.empty())
	{
		m_strList.push_back(code);
		if(m_strList.size() > 32)
			m_strList.pop_front();
		m_strIter = m_strList.end();
		m_Edit->SetText(L"");
		m_Panel->AddLine(code, m_Edit->m_Skin->m_TextColor);
		Game::getSingleton().ExecuteCode(wstou8(code).c_str());
	}
}

void Console::OnEventKeyUp(EventArgs * args)
{
	if(m_strIter != m_strList.begin())
	{
		m_Edit->SetText(*--m_strIter);
	}
}

void Console::OnEventKeyDown(EventArgs * args)
{
	if(m_strIter != m_strList.end() && ++std::list<std::wstring>::iterator(m_strIter) != m_strList.end())
	{
		m_Edit->SetText(*++m_strIter);
	}
}
