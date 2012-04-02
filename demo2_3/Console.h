
#pragma once

#include <myD3dLib.h>

class MessagePanel
	: public my::Control
{
public:
	MessagePanel(void);

	~MessagePanel(void);

	virtual void OnRender(IDirect3DDevice9 * pd3dDevice, float fElapsedTime, const my::Vector2 & Offset);
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
};

typedef boost::shared_ptr<Console> ConsolePtr;
