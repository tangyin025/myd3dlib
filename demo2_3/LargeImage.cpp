#include "stdafx.h"
#include "LargeImage.h"

LargeImageChunk::~LargeImageChunk(void)
{
	if (IsRequested())
	{
		/*_ASSERT(false);*/ ReleaseResource();
	}
}

void LargeImageChunk::RequestResource(void)
{
	_ASSERT(m_Owner);
	m_Requested = true;
	char path[MAX_PATH];
	sprintf_s(path, _countof(path), "texture/New Project Bitmap Output-256_x%d_y%d.png", m_Row, m_Col);
	my::ResourceMgr::getSingleton().LoadTextureAsync(path,
		boost::bind(&LargeImageChunk::OnTextureReady, this, boost::placeholders::_1));
}

void LargeImageChunk::ReleaseResource(void)
{
	_ASSERT(m_Owner);
	m_Requested = false;
	char path[MAX_PATH];
	sprintf_s(path, _countof(path), "texture/New Project Bitmap Output-256_x%d_y%d.png", m_Row, m_Col);
	my::ResourceMgr::getSingleton().RemoveIORequestCallback(path,
		boost::bind(&LargeImageChunk::OnTextureReady, this, boost::placeholders::_1));
	m_Texture.reset();
}

void LargeImageChunk::OnTextureReady(my::DeviceResourceBasePtr res)
{
	m_Texture = boost::dynamic_pointer_cast<my::Texture2D>(res);
}

LargeImage::LargeImage(void)
{
	m_Chunks.resize(boost::extents[2][2]);
	for (int i = 0; i < m_Chunks.shape()[0]; i++)
	{
		for (int j = 0; j < m_Chunks.shape()[1]; j++)
		{
			m_Chunks[i][j].m_Owner = this;
			m_Chunks[i][j].m_Row = i;
			m_Chunks[i][j].m_Col = j;
		}
	}
}

void LargeImage::RequestResource(void)
{

}

void LargeImage::ReleaseResource(void)
{
	ChunkSet::iterator chunk_iter = m_ViewedChunks.begin();
	for (; chunk_iter != m_ViewedChunks.end(); chunk_iter++)
	{
		chunk_iter->ReleaseResource();
	}
	m_ViewedChunks.clear();
}

void LargeImage::Draw(my::UIRender* ui_render, const my::Rectangle& rect, DWORD color, const my::Rectangle& clip)
{
	const my::Vector2 ChunkSize(rect.Width() / m_Chunks.shape()[0], rect.Height() / m_Chunks.shape()[1]);

	const int ibegin = my::Max(0, (int)floorf((clip.l - rect.l) / ChunkSize.x));

	const int jbegin = my::Max(0, (int)floorf((clip.t - rect.t) / ChunkSize.y));

	const int iend = my::Min((int)m_Chunks.shape()[0], (int)ceilf((clip.r - rect.l) / ChunkSize.x));

	const int jend = my::Min((int)m_Chunks.shape()[0], (int)ceilf((clip.b - rect.t) / ChunkSize.y));

	ChunkSet::iterator insert_chunk_iter = m_ViewedChunks.begin();
	for (int i = ibegin; i < iend; i++)
	{
		for (int j = jbegin; j < jend; j++)
		{
			if (!m_Chunks[i][j].is_linked())
			{
				_ASSERT(!m_Chunks[i][j].IsRequested());

				m_Chunks[i][j].RequestResource();

				m_ViewedChunks.insert(insert_chunk_iter, m_Chunks[i][j]);
			}
			else
			{
				ChunkSet::iterator chunk_iter = m_ViewedChunks.iterator_to(m_Chunks[i][j]);
				if (chunk_iter != insert_chunk_iter)
				{
					m_ViewedChunks.erase(chunk_iter);

					m_ViewedChunks.insert(insert_chunk_iter, m_Chunks[i][j]);
				}
				else
				{
					_ASSERT(insert_chunk_iter != m_ViewedChunks.end());

					insert_chunk_iter++;
				}

				if (m_Chunks[i][j].m_Texture)
				{
					my::Rectangle Rect(rect.l + i * ChunkSize.x, rect.t + j * ChunkSize.y, rect.l + (i + 1) * ChunkSize.x, rect.t + (j + 1) * ChunkSize.y);

					ui_render->PushRectangle(Rect, my::Rectangle(0, 0, 1, 1), color, m_Chunks[i][j].m_Texture.get(), my::UIRender::UILayerTexture, clip);
				}
			}
		}
	}

	ChunkSet::iterator chunk_iter = insert_chunk_iter;
	for (; chunk_iter != m_ViewedChunks.end(); )
	{
		chunk_iter->ReleaseResource();

		chunk_iter = m_ViewedChunks.erase(chunk_iter);
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
