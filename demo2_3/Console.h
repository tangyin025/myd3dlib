#pragma once

class MessagePanel
	: public my::Control
{
public:
	struct Line
	{
		std::wstring m_Text;

		D3DCOLOR m_Color;
	};

	Line m_lines[256];

	int m_lbegin;

	int m_lend;

	my::CriticalSection m_linesSec;

	my::ScrollBarPtr m_scrollbar;

public:
	MessagePanel(void);

	~MessagePanel(void);

	virtual void Draw(my::UIRender * ui_render, float fElapsedTime, const my::Vector2 & Offset, const my::Vector2 & Size);

	virtual bool HandleMouse(UINT uMsg, const my::Vector2 & pt, WPARAM wParam, LPARAM lParam);

	virtual bool CanHaveFocus(void);

	int MoveLineIndex(int index, int step);

	int LineIndexDistance(int start, int end);

	void _update_scrollbar(void);

	void _push_enter(D3DCOLOR Color);

	void _push_line(const std::wstring & str, D3DCOLOR Color);

	void AddLine(const std::wstring & str, D3DCOLOR Color = D3DCOLOR_ARGB(255,255,255,255));

	void puts(const std::wstring & str);
};

typedef boost::shared_ptr<MessagePanel> MessagePanelPtr;

class ConsoleEditBox
	: public my::ImeEditBox
{
public:
	my::EventFunction m_EventKeyUp;

	my::EventFunction m_EventKeyDown;

	my::EventFunction m_EventPageUp;

	my::EventFunction m_EventPageDown;

	ConsoleEditBox(void);

	~ConsoleEditBox(void);

	virtual bool HandleKeyboard(UINT uMsg, WPARAM wParam, LPARAM lParam);
};

typedef boost::shared_ptr<ConsoleEditBox> ConsoleEditBoxPtr;

class Console
	: public my::Dialog
{
public:
	my::Vector4 m_Border;

	ConsoleEditBoxPtr m_Edit;

	MessagePanelPtr m_Panel;

	std::list<std::wstring> m_strList;

	std::list<std::wstring>::iterator m_strIter;

public:
	Console(void);

	~Console(void);

	void OnEventEnter(my::EventArg * arg);

	void OnEventKeyUp(my::EventArg * arg);

	void OnEventKeyDown(my::EventArg * arg);

	void OnEventPageUp(my::EventArg * arg);

	void OnEventPageDown(my::EventArg * arg);

	void OnEventLog(const char * str);
};

typedef boost::shared_ptr<Console> ConsolePtr;
