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
	my::ScrollBarPtr scrollbar = m_scrollbar.lock();
	_ASSERT(scrollbar);
	scrollbar->m_nStart = 0;
	scrollbar->m_nEnd = LineIndexDistance(m_lbegin, m_lend);
	scrollbar->m_nPageSize = (int)(m_Size.y / m_Skin->m_Font->m_LineHeight);
	scrollbar->m_nPosition = scrollbar->m_nEnd - scrollbar->m_nPageSize;
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
	float remain_x = m_Size.x - lend_x;
	int nCP = m_Skin->m_Font->XtoCP(pString, remain_x);
	_ASSERT(wcslen(pString) == 0 || nCP > 0);
	lend_str.insert(lend_str.length(), pString, nCP);
	m_lines[m_lend].m_Color = Color;

	if(pString[nCP])
	{
		_push_enter();
		_push_sline(&pString[nCP], Color);
	}
}

void MessagePanel::AddLine(LPCWSTR pString, D3DCOLOR Color)
{
	if(*pString && m_Skin && m_Skin->m_Font)
	{
		_push_sline(pString, Color);
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

static int lua_print(lua_State * L)
{
	int n = lua_gettop(L);  /* number of arguments */
	int i;
	lua_getglobal(L, "tostring");
	for (i=1; i<=n; i++) {
		const char *s;
		lua_pushvalue(L, -1);  /* function to be called */
		lua_pushvalue(L, i);   /* value to print */
		lua_call(L, 1, 1);
		s = lua_tostring(L, -1);  /* get result */
		if (s == NULL)
			return luaL_error(L, LUA_QL("tostring") " must return a string to "
			LUA_QL("print"));
		if (i>1) Console::getSingleton().puts(L"\t");
		Console::getSingleton().puts(mstringToWString(s));
		lua_pop(L, 1);  /* pop result */
	}
	Console::getSingleton().puts(L"\n");
	return 0;
}

static int lua_exit(lua_State * L)
{
	HWND hwnd = my::DxutApp::getSingleton().GetHWND();
	_ASSERT(NULL != hwnd);
	SendMessage(hwnd, WM_CLOSE, 0, 0);
	return 0;
}

Console::SingleInstance * my::SingleInstance<Console>::s_ptr = NULL;

Console::Console(void)
{
	m_Color = D3DCOLOR_ARGB(197,0,0,0);
	m_Location = my::Vector2(50,95);
	m_Size = my::Vector2(700,410);
	m_Border = my::Vector4(5,5,5,5);

	Game * game = dynamic_cast<Game *>(my::DxutApp::getSingletonPtr());
	_ASSERT(game);

	m_Skin = game->m_defDlgSkin;

	const my::Vector2 edit_size(m_Size.x - m_Border.x - m_Border.z, 20);
	m_edit = my::ImeEditBoxPtr(new my::ImeEditBox());
	m_edit->m_Color = D3DCOLOR_ARGB(15,255,255,255);
	m_edit->m_Location = my::Vector2(m_Border.x, m_Size.y - m_Border.w - edit_size.y);
	m_edit->m_Size = edit_size;
	m_edit->m_Border = my::Vector4(0,0,0,0);
	m_edit->m_Text = L"在这里输入命令";
	m_edit->m_Skin = my::EditBoxSkinPtr(new my::EditBoxSkin());
	m_edit->m_Skin->m_Font = m_Skin->m_Font;
	m_edit->m_Skin->m_TextColor = D3DCOLOR_ARGB(255,63,188,239);
	m_Controls.insert(m_edit);

	m_edit->this_ptr = m_edit;
	m_edit->EventEnter = fastdelegate::MakeDelegate(this, &Console::OnExecute);

	const my::Vector2 scroll_size(20, m_Size.y - m_Border.y - m_edit->m_Size.y - m_Border.w);
	m_scrollbar = my::ScrollBarPtr(new my::ScrollBar());
	m_scrollbar->m_Color = D3DCOLOR_ARGB(15,255,255,255);
	m_scrollbar->m_Location = my::Vector2(m_Size.x - m_Border.z - scroll_size.x, m_Border.y);
	m_scrollbar->m_Size = scroll_size;
	m_scrollbar->m_nPageSize = 3;
	m_scrollbar->m_Skin = my::ScrollBarSkinPtr(new my::ScrollBarSkin());
	m_Controls.insert(m_scrollbar);

	const my::Vector2 panel_size(
		m_Size.x - m_Border.x - m_scrollbar->m_Size.x - m_Border.z, m_Size.y - m_Border.y - m_edit->m_Size.y - m_Border.w);
	m_panel = MessagePanelPtr(new MessagePanel());
	m_panel->m_scrollbar = m_scrollbar;
	m_panel->m_Color = D3DCOLOR_ARGB(0,0,0,0);
	m_panel->m_Location = my::Vector2(m_Border.x, m_Border.y);
	m_panel->m_Size = panel_size;
	m_panel->m_Skin = m_Skin;
	m_Controls.insert(m_panel);

	m_lua = my::LuaContextPtr(new my::LuaContext());

	lua_pushcfunction(m_lua->_state, lua_print);
	lua_setglobal(m_lua->_state, "print");

	lua_pushcfunction(m_lua->_state, lua_exit);
	lua_setglobal(m_lua->_state, "exit");
}

Console::~Console(void)
{
}

void Console::OnExecute(my::ControlPtr ctrl)
{
	my::EditBoxPtr edit = boost::dynamic_pointer_cast<my::EditBox, my::Control>(ctrl);
	_ASSERT(edit);

	AddLine(edit->m_Text.c_str(), D3DCOLOR_ARGB(255,63,188,239));

	Execute(wstringToMString(edit->m_Text.c_str()));

	edit->m_Text.clear();
	edit->m_nCaret = 0;
	edit->m_nFirstVisible = 0;
}

void Console::Execute(const std::string & cmd)
{
	try
	{
		m_lua->executeCode(cmd);
	}
	catch(const std::runtime_error & e)
	{
		AddLine(mstringToWString(e.what()).c_str());
	}
}

void Console::AddLine(LPCWSTR pString, D3DCOLOR Color)
{
	m_panel->AddLine(pString, Color);
}

void Console::puts(const std::wstring & str, D3DCOLOR Color)
{
	m_panel->puts(str, Color);
}
