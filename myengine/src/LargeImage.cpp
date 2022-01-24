#include "LargeImage.h"
#include "myResource.h"
#include <boost/tuple/tuple_comparison.hpp>
//#include <boost/functional/hash.hpp>
//
//namespace boost {
//	namespace tuples {
//		namespace detail {
//			template <class Tuple, size_t Index = length<Tuple>::value - 1>
//			struct HashValueImpl
//			{
//				static void apply(size_t& seed, Tuple const& tuple)
//				{
//					HashValueImpl<Tuple, Index - 1>::apply(seed, tuple);
//					boost::hash_combine(seed, tuple.get<Index>());
//				}
//			};
//
//			template <class Tuple>
//			struct HashValueImpl<Tuple, 0>
//			{
//				static void apply(size_t& seed, Tuple const& tuple)
//				{
//					boost::hash_combine(seed, tuple.get<0>());
//				}
//			};
//		} // namespace detail
//
//		template <class Tuple>
//		size_t hash_value(Tuple const& tuple)
//		{
//			size_t seed = 0;
//			detail::HashValueImpl<Tuple>::apply(seed, tuple);
//			return seed;
//		}
//	}
//}

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
		sprintf_s(path, _countof(path), m_Owner->m_TexturePath.c_str(), m_Depth, m_Row, m_Col);
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
		sprintf_s(path, _countof(path), m_Owner->m_TexturePath.c_str(), m_Depth, m_Row, m_Col);
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

void LargeImage::Draw(my::UIRender * ui_render, const my::Rectangle & rect, DWORD color, const my::Rectangle & clip, int depth)
{
	const int row_count = my::ipow(2, depth);
	const int col_count = my::ipow(2, depth);
	const my::Vector2 ChunkSize(rect.Width() / row_count, rect.Height() / col_count);
	const int ibegin = my::Max(0, (int)floorf((clip.l - rect.l) / ChunkSize.x));
	const int jbegin = my::Max(0, (int)floorf((clip.t - rect.t) / ChunkSize.y));
	const int iend = my::Min(row_count, (int)ceilf((clip.r - rect.l) / ChunkSize.x));
	const int jend = my::Min(col_count, (int)ceilf((clip.b - rect.t) / ChunkSize.y));
	ChunkSet::iterator insert_chunk_iter = m_ViewedChunks.begin();
	for (int i = ibegin; i < iend; i++)
	{
		for (int j = jbegin; j < jend; j++)
		{
			std::pair<LargeImageChunkMap::iterator, bool> res = m_Chunks.insert(LargeImageChunkMap::value_type(boost::make_tuple(depth, i, j), LargeImageChunk(this, depth, i, j)));

			if (res.first->second.is_linked())
			{
				ChunkSet::iterator chunk_iter = m_ViewedChunks.iterator_to(res.first->second);
				if (chunk_iter != insert_chunk_iter)
				{
					m_ViewedChunks.erase(chunk_iter);

					m_ViewedChunks.insert(insert_chunk_iter, res.first->second);
				}
				else
				{
					_ASSERT(insert_chunk_iter != m_ViewedChunks.end());

					insert_chunk_iter++;
				}

				if (res.first->second.m_Texture)
				{
					my::Rectangle Rect(rect.l + i * ChunkSize.x, rect.t + j * ChunkSize.y, rect.l + (i + 1) * ChunkSize.x, rect.t + (j + 1) * ChunkSize.y);

					ui_render->PushRectangle(Rect, my::Rectangle(0, 0, 1, 1), color, res.first->second.m_Texture.get(), clip);
				}
			}
			else
			{
				_ASSERT(!res.first->second.IsRequested());

				res.first->second.RequestResource();

				m_ViewedChunks.insert(insert_chunk_iter, res.first->second);
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
