#include "stdafx.h"
#include "Console.h"
#include "Client.h"
#include <boost/algorithm/string.hpp>

using namespace my;

MessagePanel::MessagePanel(void)
	: m_lbegin(0)
	, m_lend(0)
{
	m_scrollbar.reset(new ScrollBar(NULL));

	InsertControl(m_scrollbar);
}

MessagePanel::~MessagePanel(void)
{
}

void MessagePanel::Draw(UIRender * ui_render, float fElapsedTime, const my::Vector2 & Offset, const my::Vector2 & Size)
{
	Control::Draw(ui_render, fElapsedTime, Offset, Size);

	if(m_Skin && m_Skin->m_Font)
	{
		int i = MoveLineIndex(m_lbegin, m_scrollbar->m_nPosition);
		float y = m_Rect.t;
		for(; i != m_lend && y <= m_Rect.b - m_Skin->m_Font->m_LineHeight; i = MoveLineIndex(i, 1), y += m_Skin->m_Font->m_LineHeight)
		{
			m_Skin->m_Font->PushString(ui_render, m_lines[i].m_Text.c_str(),
				my::Rectangle(m_Rect.l, y, m_Rect.r, y + m_Skin->m_Font->m_LineHeight), m_lines[i].m_Color);
		}
	}
}

bool MessagePanel::HandleMouse(UINT uMsg, const my::Vector2 & pt, WPARAM wParam, LPARAM lParam)
{
	if (uMsg == WM_MOUSEWHEEL && m_scrollbar->HandleMouse(uMsg, pt, wParam, lParam))
	{
		return true;
	}
	return false;
}

bool MessagePanel::CanHaveFocus(void)
{
	return false;
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
	m_scrollbar->m_nPageSize = (int)(m_Height.offset / m_Skin->m_Font->m_LineHeight);
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
		float remain_x = m_Width.offset - m_scrollbar->m_Width.offset - lend_x;
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
		puts(str);
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
				if(m_EventKeyUp)
					m_EventKeyUp(&ControlEventArg(this));
				ResetCaretBlink();
				return true;

			case VK_DOWN:
				if(m_EventKeyDown)
					m_EventKeyDown(&ControlEventArg(this));
				ResetCaretBlink();
				return true;

			case VK_PRIOR:
				if (m_EventPageUp)
					m_EventPageUp(&ControlEventArg(this));
				return true;

			case VK_NEXT:
				if (m_EventPageDown)
					m_EventPageDown(&ControlEventArg(this));
				return true;
			}
			break;
		}
	}
	return ImeEditBox::HandleKeyboard(uMsg, wParam, lParam);
}

Console::Console(void)
{
	m_x = UDim(0, 50);
	m_y = UDim(0, 95);
	m_Width = UDim(0, 700);
	m_Height = UDim(0, 410);
	m_Skin.reset(new ControlSkin());
	m_Skin->m_Color = D3DCOLOR_ARGB(197,0,0,0);
	m_Skin->m_Font = Client::getSingleton().m_Font;
	m_Skin->m_TextColor = D3DCOLOR_ARGB(255,255,255,255);
	m_Skin->m_TextAlign = Font::AlignLeftTop;
	m_Skin->m_Image.reset(new ControlImage());
	m_Skin->m_Image->m_Texture = my::ResourceMgr::getSingleton().LoadTexture("texture/CommonUI.png");
	m_Skin->m_Image->m_Rect = my::Rectangle::LeftTop(158,43,2,2);
	m_Skin->m_Image->m_Border = Vector4(0,0,0,0);

	const Vector4 Border(5,5,5,5);

	m_Edit.reset(new ConsoleEditBox());
	m_Edit->m_Width = UDim(0, m_Width.offset - Border.x - Border.z);
	m_Edit->m_Height = UDim(0, 20);
	m_Edit->m_x = UDim(0, Border.x);
	m_Edit->m_y = UDim(0, m_Height.offset - Border.w - m_Edit->m_Height.offset);
	m_Edit->m_Border = Vector4(0,0,0,0);
	m_Edit->m_Skin.reset(new EditBoxSkin());
	m_Edit->m_Skin->m_Color = D3DCOLOR_ARGB(15,255,255,255);
	m_Edit->m_Skin->m_Font = Client::getSingleton().m_Font;
	m_Edit->m_Skin->m_TextColor = D3DCOLOR_ARGB(255,63,188,239);
	m_Edit->m_Skin->m_TextAlign = Font::AlignLeftMiddle;
	m_Edit->m_Skin->m_Image.reset(new ControlImage());
	m_Edit->m_Skin->m_Image->m_Texture = my::ResourceMgr::getSingleton().LoadTexture("texture/CommonUI.png");
	m_Edit->m_Skin->m_Image->m_Rect = my::Rectangle::LeftTop(158,43,2,2);
	m_Edit->m_Skin->m_Image->m_Border = Vector4(0,0,0,0);
	boost::dynamic_pointer_cast<EditBoxSkin>(m_Edit->m_Skin)->m_FocusedImage.reset(new ControlImage());
	boost::dynamic_pointer_cast<EditBoxSkin>(m_Edit->m_Skin)->m_FocusedImage->m_Texture = my::ResourceMgr::getSingleton().LoadTexture("texture/CommonUI.png");
	boost::dynamic_pointer_cast<EditBoxSkin>(m_Edit->m_Skin)->m_FocusedImage->m_Rect = my::Rectangle::LeftTop(158,43,2,2);
	boost::dynamic_pointer_cast<EditBoxSkin>(m_Edit->m_Skin)->m_FocusedImage->m_Border = Vector4(0,0,0,0);
	boost::dynamic_pointer_cast<EditBoxSkin>(m_Edit->m_Skin)->m_SelBkColor = D3DCOLOR_ARGB(255,255,128,0);
	boost::dynamic_pointer_cast<EditBoxSkin>(m_Edit->m_Skin)->m_CaretImage.reset(new ControlImage());
	boost::dynamic_pointer_cast<EditBoxSkin>(m_Edit->m_Skin)->m_CaretImage->m_Texture = my::ResourceMgr::getSingleton().LoadTexture("texture/CommonUI.png");
	boost::dynamic_pointer_cast<EditBoxSkin>(m_Edit->m_Skin)->m_CaretImage->m_Rect = my::Rectangle::LeftTop(158,43,2,2);
	boost::dynamic_pointer_cast<EditBoxSkin>(m_Edit->m_Skin)->m_CaretImage->m_Border = Vector4(0,0,0,0);
	m_Edit->m_EventEnter = boost::bind(&Console::OnEventEnter, this, _1);
	m_Edit->m_EventKeyUp = boost::bind(&Console::OnEventKeyUp, this, _1);
	m_Edit->m_EventKeyDown = boost::bind(&Console::OnEventKeyDown, this, _1);
	m_Edit->m_EventPageUp = boost::bind(&Console::OnEventPageUp, this, _1);
	m_Edit->m_EventPageDown = boost::bind(&Console::OnEventPageDown, this, _1);
	InsertControl(m_Edit);

	m_Panel.reset(new MessagePanel());
	m_Panel->m_Width = UDim(0, m_Width.offset - Border.x - Border.z);
	m_Panel->m_Height = UDim(0, m_Height.offset - Border.y - Border.w - m_Edit->m_Height.offset);
	m_Panel->m_x = UDim(0, Border.x);
	m_Panel->m_y = UDim(0, Border.y);
	m_Panel->m_Skin.reset(new ControlSkin());
	m_Panel->m_Skin->m_Color = D3DCOLOR_ARGB(0,0,0,0);
	m_Panel->m_Skin->m_Font = Client::getSingleton().m_Font;
	m_Panel->m_scrollbar->m_Width = UDim(0, 20);
	m_Panel->m_scrollbar->m_Height = UDim(0, m_Panel->m_Height.offset);
	m_Panel->m_scrollbar->m_x = UDim(0, m_Panel->m_Width.offset - m_Panel->m_scrollbar->m_Width.offset);
	m_Panel->m_scrollbar->m_y = UDim(0, 0);
	m_Panel->m_scrollbar->m_nPageSize = 3;
	m_Panel->m_scrollbar->m_Skin.reset(new ScrollBarSkin());
	m_Panel->m_scrollbar->m_Skin->m_Color = D3DCOLOR_ARGB(15,255,255,255);
	m_Panel->m_scrollbar->m_Skin->m_Image.reset(new ControlImage());
	m_Panel->m_scrollbar->m_Skin->m_Image->m_Texture = my::ResourceMgr::getSingleton().LoadTexture("texture/CommonUI.png");
	m_Panel->m_scrollbar->m_Skin->m_Image->m_Rect = my::Rectangle::LeftTop(158,43,2,2);
	m_Panel->m_scrollbar->m_Skin->m_Image->m_Border = Vector4(0,0,0,0);
	boost::dynamic_pointer_cast<ScrollBarSkin>(m_Panel->m_scrollbar->m_Skin)->m_UpBtnNormalImage.reset(new ControlImage());
	boost::dynamic_pointer_cast<ScrollBarSkin>(m_Panel->m_scrollbar->m_Skin)->m_UpBtnNormalImage->m_Texture = my::ResourceMgr::getSingleton().LoadTexture("texture/CommonUI.png");
	boost::dynamic_pointer_cast<ScrollBarSkin>(m_Panel->m_scrollbar->m_Skin)->m_UpBtnNormalImage->m_Rect = my::Rectangle::LeftTop(158,43,2,2);
	boost::dynamic_pointer_cast<ScrollBarSkin>(m_Panel->m_scrollbar->m_Skin)->m_UpBtnNormalImage->m_Border = Vector4(0,0,0,0);
	boost::dynamic_pointer_cast<ScrollBarSkin>(m_Panel->m_scrollbar->m_Skin)->m_DownBtnNormalImage.reset(new ControlImage());
	boost::dynamic_pointer_cast<ScrollBarSkin>(m_Panel->m_scrollbar->m_Skin)->m_DownBtnNormalImage->m_Texture = my::ResourceMgr::getSingleton().LoadTexture("texture/CommonUI.png");
	boost::dynamic_pointer_cast<ScrollBarSkin>(m_Panel->m_scrollbar->m_Skin)->m_DownBtnNormalImage->m_Rect = my::Rectangle::LeftTop(158,43,2,2);
	boost::dynamic_pointer_cast<ScrollBarSkin>(m_Panel->m_scrollbar->m_Skin)->m_DownBtnNormalImage->m_Border = Vector4(0,0,0,0);
	boost::dynamic_pointer_cast<ScrollBarSkin>(m_Panel->m_scrollbar->m_Skin)->m_ThumbBtnNormalImage.reset(new ControlImage());
	boost::dynamic_pointer_cast<ScrollBarSkin>(m_Panel->m_scrollbar->m_Skin)->m_ThumbBtnNormalImage->m_Texture = my::ResourceMgr::getSingleton().LoadTexture("texture/CommonUI.png");
	boost::dynamic_pointer_cast<ScrollBarSkin>(m_Panel->m_scrollbar->m_Skin)->m_ThumbBtnNormalImage->m_Rect = my::Rectangle::LeftTop(158,43,2,2);
	boost::dynamic_pointer_cast<ScrollBarSkin>(m_Panel->m_scrollbar->m_Skin)->m_ThumbBtnNormalImage->m_Border = Vector4(0,0,0,0);
	InsertControl(m_Panel);

	m_strIter = m_strList.end();
	Client::getSingleton().m_EventLog.connect(boost::bind(&Console::OnEventLog, this, _1));
}

Console::~Console(void)
{
	Client::getSingleton().m_EventLog.disconnect(boost::bind(&Console::OnEventLog, this, _1));
}

void Console::OnEventEnter(EventArg * arg)
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
		Client::getSingleton().ExecuteCode(wstou8(code).c_str());
	}
}

void Console::OnEventKeyUp(EventArg * arg)
{
	if(m_strIter != m_strList.begin())
	{
		m_Edit->SetText(*--m_strIter);
	}
}

void Console::OnEventKeyDown(EventArg * arg)
{
	if(m_strIter != m_strList.end() && ++std::list<std::wstring>::iterator(m_strIter) != m_strList.end())
	{
		m_Edit->SetText(*++m_strIter);
	}
}

void Console::OnEventPageUp(my::EventArg * arg)
{
	m_Panel->m_scrollbar->Scroll(-m_Panel->m_scrollbar->m_nPageSize);
}

void Console::OnEventPageDown(my::EventArg * arg)
{
	m_Panel->m_scrollbar->Scroll( m_Panel->m_scrollbar->m_nPageSize);
}

void Console::OnEventLog(const char * str)
{
	std::wstring logs = ms2ws(str);
	boost::trim_if(logs, boost::algorithm::is_any_of(L"\n\r"));
	m_Panel->AddLine(logs);
}
