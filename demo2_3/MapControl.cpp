#include "stdafx.h"
#include "MapControl.h"

MapControl::MapControl(const char* Name)
	: Control(Name)
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

		const my::Vector2 ChunkSize(WidthHeight.x / m_Textures.shape()[0], WidthHeight.y / m_Textures.shape()[1]);

		const int ibegin = my::Max(0, (int)floorf((Offset.x - LeftTop.x) / ChunkSize.x));

		const int jbegin = my::Max(0, (int)floorf((Offset.y - LeftTop.y) / ChunkSize.y));

		const int iend = my::Min((int)m_Textures.shape()[0], (int)ceilf((Offset.x + Size.x - LeftTop.x) / ChunkSize.x));

		const int jend = my::Min((int)m_Textures.shape()[0], (int)ceilf((Offset.y + Size.y - LeftTop.y) / ChunkSize.y));

		for (int i = ibegin; i < iend; i++)
		{
			for (int j = jbegin; j < jend; j++)
			{
				if (m_Textures[i][j])
				{
					my::Rectangle Rect(i * ChunkSize.x + LeftTop.x, j * ChunkSize.y + LeftTop.y, (i + 1) * ChunkSize.x + LeftTop.x, (j + 1) * ChunkSize.y + LeftTop.y);

					ui_render->PushRectangle(Rect, my::Rectangle(0, 0, 1, 1), m_Skin->m_Color, m_Textures[i][j].get(), my::UIRender::UILayerTexture);
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
