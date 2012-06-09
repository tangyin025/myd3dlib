
#pragma once

#include <myD3dLib.h>

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

	my::ScrollBarPtr m_scrollbar;

public:
	MessagePanel(void);

	~MessagePanel(void);

	virtual void OnRender(IDirect3DDevice9 * pd3dDevice, float fElapsedTime, const my::Vector2 & Offset);

	virtual bool MsgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

	virtual bool HandleKeyboard(UINT uMsg, WPARAM wParam, LPARAM lParam);

	virtual bool HandleMouse(UINT uMsg, const my::Vector2 & pt, WPARAM wParam, LPARAM lParam);

	virtual bool CanHaveFocus(void);

	int MoveLineIndex(int index, int step);

	int LineIndexDistance(int start, int end);

	void _update_scrollbar(void);

	void _push_enter(void);

	void _push_sline(LPCWSTR pString, D3DCOLOR Color);

	void AddLine(const std::wstring & str, D3DCOLOR Color = D3DCOLOR_ARGB(255,255,255,255));

	void puts(const std::wstring & str, D3DCOLOR Color = D3DCOLOR_ARGB(255,255,255,255));
};

typedef boost::shared_ptr<MessagePanel> MessagePanelPtr;

class ConsoleImeEditBox
	: public my::ImeEditBox
{
public:
	my::ControlEvent EventPrevLine;

	my::ControlEvent EventNextLine;

	ConsoleImeEditBox(void);

	~ConsoleImeEditBox(void);

	virtual bool HandleKeyboard(UINT uMsg, WPARAM wParam, LPARAM lParam);
};
//
//class Console
//	: public my::Dialog
//{
//public:
//	my::Vector4 m_Border;
//
//	my::ImeEditBoxPtr m_edit;
//
//	MessagePanelPtr m_panel;
//
//public:
//	Console(void);
//
//	~Console(void);
//};
//
//typedef boost::shared_ptr<Console> ConsolePtr;
