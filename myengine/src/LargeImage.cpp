#include "LargeImage.h"
#include "myResource.h"

LargeImageChunk::~LargeImageChunk(void)
{
	if (IsRequested())
	{
		_ASSERT(false); ReleaseResource();
	}
}

void LargeImageChunk::RequestResource(void)
{
	m_Requested = true;

	_ASSERT(m_Owner);

	if (!m_Owner->m_TexturePath.empty())
	{
		_ASSERT(!m_Texture);

		char path[MAX_PATH];
		sprintf_s(path, _countof(path), m_Owner->m_TexturePath.c_str(), m_Row, m_Col);
		my::ResourceMgr::getSingleton().LoadTextureAsync(path,
			boost::bind(&LargeImageChunk::OnTextureReady, this, boost::placeholders::_1));
	}
}

void LargeImageChunk::ReleaseResource(void)
{
	m_Requested = false;

	_ASSERT(m_Owner);

	if (!m_Owner->m_TexturePath.empty())
	{
		char path[MAX_PATH];
		sprintf_s(path, _countof(path), m_Owner->m_TexturePath.c_str(), m_Row, m_Col);
		my::ResourceMgr::getSingleton().RemoveIORequestCallback(path,
			boost::bind(&LargeImageChunk::OnTextureReady, this, boost::placeholders::_1));

		m_Texture.reset();
	}
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

void LargeImage::Draw(my::UIRender * ui_render, const my::Rectangle & rect, DWORD color, const my::Rectangle & clip)
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
			if (m_Chunks[i][j].is_linked())
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

					ui_render->PushRectangle(Rect, my::Rectangle(0, 0, 1, 1), color, m_Chunks[i][j].m_Texture.get(), clip);
				}
			}
			else
			{
				_ASSERT(!m_Chunks[i][j].IsRequested());

				m_Chunks[i][j].RequestResource();

				m_ViewedChunks.insert(insert_chunk_iter, m_Chunks[i][j]);
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
