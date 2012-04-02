
#pragma once

#include <myD3dLib.h>
#define BOOST_TR1_USE_OLD_TUPLE
#include <boost/tr1/array.hpp>

class MessagePanel
	: public my::Control
{
public:
	struct Line
	{
		std::wstring m_Text;

		D3DCOLOR m_Color;
	};

	typedef boost::array<Line, 256> LineArray;

	LineArray m_lines;

	int m_lbegin;

	int m_lend;

	boost::weak_ptr<my::ScrollBar> m_scrollbar;

public:
	MessagePanel(void);

	~MessagePanel(void);

	virtual void OnRender(IDirect3DDevice9 * pd3dDevice, float fElapsedTime, const my::Vector2 & Offset);

	static int MoveLineIndex(int index, int step);

	static int LineIndexDistance(int start, int end);

	void AddLine(LPCWSTR pString, D3DCOLOR Color = D3DCOLOR_ARGB(255,255,255,255));
};

typedef boost::shared_ptr<MessagePanel> MessagePanelPtr;

class Console
	: public my::Dialog
{
protected:
	my::Vector4 m_Border;

	my::ImeEditBoxPtr m_edit;

	my::ScrollBarPtr m_scrollbar;

	MessagePanelPtr m_panel;

public:
	Console(void);

	~Console(void);

	void OnExecute(my::ControlPtr ctrl);
};

typedef boost::shared_ptr<Console> ConsolePtr;
