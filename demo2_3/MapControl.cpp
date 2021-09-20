#include "stdafx.h"
#include "MapControl.h"

LargeImageChunk::~LargeImageChunk(void)
{
	//_ASSERT(!m_Requested);
}

LargeImage::LargeImage(void)
{
	m_Chunks.resize(boost::extents[2][2]);
}

void LargeImage::RequestResource(void)
{

}

void LargeImage::ReleaseResource(void)
{
	for (int i = 0; i < m_Chunks.shape()[0]; i++)
	{
		for (int j = 0; j < m_Chunks.shape()[1]; j++)
		{
			if (m_Chunks[i][j].m_Requested)
			{
				char buff[256];
				sprintf_s(buff, _countof(buff), "texture/New Project Bitmap Output-256_x%d_y%d.png", i, j);
				my::ResourceMgr::getSingleton().RemoveIORequestCallback(buff, boost::bind(&LargeImage::OnTextureReady, this, boost::placeholders::_1, i, j));
				m_Chunks[i][j].m_Requested = false;
			}
		}
	}
}

void LargeImage::OnTextureReady(my::DeviceResourceBasePtr res, int i, int j)
{
	_ASSERT(i >= 0 && i < m_Chunks.shape()[0] && j >= 0 && j < m_Chunks.shape()[1]);

	_ASSERT(!m_Chunks[i][j].m_Texture);

	m_Chunks[i][j].m_Texture = boost::dynamic_pointer_cast<my::Texture2D>(res);
}

void LargeImage::Draw(my::UIRender* ui_render, const my::Rectangle& rect, DWORD color, const my::Rectangle& clip)
{
	const my::Vector2 ChunkSize(rect.Width() / m_Chunks.shape()[0], rect.Height() / m_Chunks.shape()[1]);

	const int ibegin = my::Max(0, (int)floorf((clip.l - rect.l) / ChunkSize.x));

	const int jbegin = my::Max(0, (int)floorf((clip.t - rect.t) / ChunkSize.y));

	const int iend = my::Min((int)m_Chunks.shape()[0], (int)ceilf((clip.r - rect.l) / ChunkSize.x));

	const int jend = my::Min((int)m_Chunks.shape()[0], (int)ceilf((clip.b - rect.t) / ChunkSize.y));

	for (int i = ibegin; i < iend; i++)
	{
		for (int j = jbegin; j < jend; j++)
		{
			if (m_Chunks[i][j].m_Texture)
			{
				my::Rectangle Rect(rect.l + i * ChunkSize.x, rect.t + j * ChunkSize.y, rect.l + (i + 1) * ChunkSize.x, rect.t + (j + 1) * ChunkSize.y);

				ui_render->PushRectangle(Rect, my::Rectangle(0, 0, 1, 1), color, m_Chunks[i][j].m_Texture.get(), my::UIRender::UILayerTexture, clip);
			}
			else if (!m_Chunks[i][j].m_Requested)
			{
				char buff[256];
				sprintf_s(buff, _countof(buff), "texture/New Project Bitmap Output-256_x%d_y%d.png", i, j);
				my::ResourceMgr::getSingleton().LoadTextureAsync(buff, boost::bind(&LargeImage::OnTextureReady, this, boost::placeholders::_1, i, j));
				m_Chunks[i][j].m_Requested = true;
			}
		}
	}
}

MapControl::MapControl(const char* Name)
	: Control(Name)
	, m_bMouseDrag(false)
	, m_MouseOffset(0, 0)
{
}

void MapControl::Draw(my::UIRender* ui_render, float fElapsedTime, const my::Vector2& Offset, const my::Vector2& Size)
{
	if (m_Skin)
	{
		m_Rect = my::Rectangle::LeftTop(Offset.x + m_x.scale * Size.x + m_x.offset, Offset.y + m_y.scale * Size.y + m_y.offset, m_Width.scale * Size.x + m_Width.offset, m_Height.scale * Size.y + m_Height.offset);

		my::Rectangle Clip = my::Rectangle::LeftTop(Offset, Size);

		m_largeImg.Draw(ui_render, m_Rect, m_Skin->m_Color, Clip);
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
