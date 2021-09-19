#include "stdafx.h"
#include "MapControl.h"

MapControl::MapControl(const char* Name)
	: Control(Name)
	, m_bMouseDrag(false)
	, m_MouseOffset(0, 0)
{
	m_Textures.resize(boost::extents[2][2]);

	m_IsRequested.resize(boost::extents[2][2]);

	std::fill_n(m_IsRequested.data(), m_IsRequested.num_elements(), false);
}

void MapControl::OnTextureReady(my::DeviceResourceBasePtr res, int i, int j)
{
	_ASSERT(i >= 0 && i < m_Textures.shape()[0] && j >= 0 && j < m_Textures.shape()[1]);

	_ASSERT(!m_Textures[i][j]);

	m_Textures[i][j] = boost::dynamic_pointer_cast<my::Texture2D>(res);
}

void MapControl::Draw(my::UIRender* ui_render, float fElapsedTime, const my::Vector2& Offset, const my::Vector2& Size)
{
	if (m_Skin)
	{
		const my::Vector2 LeftTop(Offset.x + m_x.scale * Size.x + m_x.offset, Offset.y + m_y.scale * Size.y + m_y.offset);

		const my::Vector2 WidthHeight(m_Width.scale * Size.x + m_Width.offset, m_Height.scale * Size.y + m_Height.offset);

		m_Rect = my::Rectangle::LeftTop(LeftTop, WidthHeight);

		const my::Vector2 ChunkSize(WidthHeight.x / m_Textures.shape()[0], WidthHeight.y / m_Textures.shape()[1]);

		const my::Rectangle Clip = my::Rectangle::LeftTop(Offset, Size);

		const int ibegin = my::Max(0, (int)floorf((Clip.l - LeftTop.x) / ChunkSize.x));

		const int jbegin = my::Max(0, (int)floorf((Clip.t - LeftTop.y) / ChunkSize.y));

		const int iend = my::Min((int)m_Textures.shape()[0], (int)ceilf((Clip.r - LeftTop.x) / ChunkSize.x));

		const int jend = my::Min((int)m_Textures.shape()[0], (int)ceilf((Clip.b - LeftTop.y) / ChunkSize.y));

		for (int i = ibegin; i < iend; i++)
		{
			for (int j = jbegin; j < jend; j++)
			{
				if (m_Textures[i][j])
				{
					my::Rectangle Rect(LeftTop.x + i * ChunkSize.x, LeftTop.y + j * ChunkSize.y, LeftTop.x + (i + 1) * ChunkSize.x, LeftTop.y + (j + 1) * ChunkSize.y);

					ui_render->PushRectangle(Rect, my::Rectangle(0, 0, 1, 1), m_Skin->m_Color, m_Textures[i][j].get(), my::UIRender::UILayerTexture, Clip);
				}
				else if (!m_IsRequested[i][j])
				{
					char buff[256];
					sprintf_s(buff, _countof(buff), "texture/New Project Bitmap Output-256_x%d_y%d.png", i, j);
					my::ResourceMgr::getSingleton().LoadTextureAsync(buff, boost::bind(&MapControl::OnTextureReady, this, boost::placeholders::_1, i, j));
					m_IsRequested[i][j] = true;
				}
			}
		}
	}
}

bool MapControl::HandleMouse(UINT uMsg, const my::Vector2& pt, WPARAM wParam, LPARAM lParam)
{
	if (m_bEnabled && m_bVisible)
	{
		switch (uMsg)
		{
		case WM_LBUTTONDOWN:
		case WM_LBUTTONDBLCLK:
			if (HitTest(pt))
			{
				m_bPressed = true;
				m_MouseOffset.x = pt.x - m_x.offset;
				m_MouseOffset.y = pt.y - m_y.offset;
				if (!s_FocusControl || !ContainsControl(s_FocusControl))
				{
					SetFocusRecursive();
				}
				SetCaptureControl(this);
				return true;
			}
			break;

		case WM_LBUTTONUP:
			if (m_bPressed)
			{
				SetCaptureControl(NULL);
				m_bPressed = false;

				if (m_bMouseDrag)
				{
					m_bMouseDrag = false;
				}
				else
				{
					if (m_Skin && !m_Skin->m_MouseClickSound.empty())
					{
						my::D3DContext::getSingleton().OnControlSound(m_Skin->m_MouseClickSound.c_str());
					}

					if (m_EventMouseClick)
					{
						my::MouseEventArg arg(this, pt);
						m_EventMouseClick(&arg);
					}
				}
				return true;
			}
			break;

		case WM_MOUSEMOVE:
			if (m_bPressed)
			{
				if (!m_bMouseDrag)
				{
					m_bMouseDrag = true;
				}

				if (m_bMouseDrag)
				{
					m_x.offset = pt.x - m_MouseOffset.x;
					m_y.offset = pt.y - m_MouseOffset.y;
				}
			}
			break;
		}
	}
	return false;
}
