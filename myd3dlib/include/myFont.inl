
namespace my
{
	template <typename T, typename U>
	void Font::DrawString(const Vector2& pen, float right, LPCWSTR pString, Font::Align align, const T& get_character_info, const U& draw_character)
	{
		const wchar_t* p = pString;
		for (float x = pen.x, y = pen.y; *p; p++)
		{
			const CharacterInfo* info = get_character_info(*p);

			if (align & Font::AlignMultiLine)
			{
				if (x + info->horiAdvance > right)
				{
					x = pen.x;
					y += m_LineHeight;
				}
				else if (*p == L'\n')
				{
					x = pen.x;
					y += m_LineHeight;
					continue;
				}
			}

			draw_character(x, y, info);

			x += info->horiAdvance;
		}
	}
}
