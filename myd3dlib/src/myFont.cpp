
#include "stdafx.h"
#include "myd3dlib.h"

#ifdef _DEBUG
#define new new( _CLIENT_BLOCK, __FILE__, __LINE__ )
#endif

using namespace my;

SpritePtr Sprite::CreateSprite(LPDIRECT3DDEVICE9 pDevice)
{
	LPD3DXSPRITE pSprite = NULL;
	HRESULT hres = D3DXCreateSprite(pDevice, &pSprite);
	if(FAILED(hres))
	{
		THROW_D3DEXCEPTION(hres);
	}

	return SpritePtr(new Sprite(pSprite));
}

bool RectAssignmentNode::AssignTopRect(const SIZE & size, RECT & outRect)
{
	_ASSERT(size.cx <= m_rect.right - m_rect.left);
	_ASSERT(size.cy < m_rect.bottom - m_rect.top);

	RECT rectUp, rectDown;
	int y = m_rect.top + size.cy;
	SetRect(&rectUp, m_rect.left, m_rect.top, m_rect.right, y);
	SetRect(&rectDown, m_rect.left, y, m_rect.right, m_rect.bottom);

	_ASSERT(NULL == m_lchild);
	_ASSERT(NULL == m_rchild);

	m_lchild = RectAssignmentNodePtr(new RectAssignmentNode(rectUp));
	m_rchild = RectAssignmentNodePtr(new RectAssignmentNode(rectDown));
	bool ret = m_lchild->AssignRect(size, outRect);
	_ASSERT(ret);

	return true;
}

bool RectAssignmentNode::AssignLeftRect(const SIZE & size, RECT & outRect)
{
	_ASSERT(size.cx < m_rect.right - m_rect.left);
	_ASSERT(size.cy <= m_rect.bottom - m_rect.top);

	RECT rectLeft, rectRight;
	int x = m_rect.left + size.cx;
	SetRect(&rectLeft, m_rect.left, m_rect.top, x, m_rect.bottom);
	SetRect(&rectRight, x, m_rect.top, m_rect.right, m_rect.bottom);

	_ASSERT(NULL == m_lchild);
	_ASSERT(NULL == m_rchild);

	m_lchild = RectAssignmentNodePtr(new RectAssignmentNode(rectLeft));
	m_rchild = RectAssignmentNodePtr(new RectAssignmentNode(rectRight));
	bool ret = m_lchild->AssignRect(size, outRect);
	_ASSERT(ret);

	return true;
}

bool RectAssignmentNode::AssignRect(const SIZE & size, RECT & outRect)
{
	//_ASSERT(size.cx > 0 && size.cy > 0);

	if(m_used)
	{
		return false;
	}

	if(NULL != m_lchild)
	{
		if(!m_lchild->AssignRect(size, outRect))
		{
			_ASSERT(m_rchild);
			return m_rchild->AssignRect(size, outRect);
		}
		return true;
	}

	int width = m_rect.right - m_rect.left;
	int height = m_rect.bottom - m_rect.top;
	if(width == size.cx)
	{
		if(height == size.cy)
		{
			m_used = true;
			outRect = m_rect;
			return true;
		}
		else if(height > size.cy)
		{
			return AssignTopRect(size, outRect);
		}
	}
	else if(width > size.cx)
	{
		if(height == size.cy)
		{
			return AssignLeftRect(size, outRect);
		}
		else if(height > size.cy)
		{
			if(width > height)
			{
				return AssignLeftRect(size, outRect);
			}
			else
			{
				return AssignTopRect(size, outRect);
			}
		}
	}

	return false;
}

Font::Font(FT_Face face, int height, LPDIRECT3DDEVICE9 pDevice, unsigned short pixel_gap)
	: m_face(face)
	, m_Device(pDevice)
	, FONT_PIXEL_GAP(pixel_gap)
{
	_ASSERT(m_face);

	FT_Error err = FT_Set_Pixel_Sizes(m_face, height, height);
	if(err)
	{
		THROW_CUSEXCEPTION("FT_Set_Pixel_Sizes failed");
	}

	m_LineHeight = m_face->size->metrics.height >> 6;

	m_maxAdvance = m_face->size->metrics.max_advance >> 6;

	CreateFontTexture(256, 256);

	m_textureRectRoot = RectAssignmentNodePtr(new RectAssignmentNode(CRect(0, 0, m_textureDesc.Width, m_textureDesc.Height)));
}

Font::~Font(void)
{
	FT_Done_Face(m_face);
}

FontPtr Font::CreateFontFromFile(
	LPDIRECT3DDEVICE9 pDevice,
	LPCSTR pFilename,
	float height,
	unsigned short pixel_gap,
	FT_Long face_index)
{
	FT_Face face;
	FT_Error err = FT_New_Face(ResourceMgr::getSingleton().m_library, pFilename, face_index, &face);
	if(err)
	{
		THROW_CUSEXCEPTION("FT_New_Face failed");
	}

	FontPtr font(new Font(face, height, pDevice, pixel_gap));
	return font;
}

FontPtr Font::CreateFontFromFileInMemory(
	LPDIRECT3DDEVICE9 pDevice,
	const void * file_base,
	long file_size,
	float height,
	unsigned short pixel_gap,
	FT_Long face_index)
{
	boost::shared_array<unsigned char> cache(new unsigned char[file_size]);
	memcpy(cache.get(), file_base, file_size);

	FT_Face face;
	FT_Error err = FT_New_Memory_Face(ResourceMgr::getSingleton().m_library, cache.get(), file_size, face_index, &face);
	if(err)
	{
		THROW_CUSEXCEPTION("FT_New_Memory_Face failed");
	}

	FontPtr font(new Font(face, height, pDevice, pixel_gap));
	font->m_cache = cache;
	return font;
}

void Font::OnResetDevice(void)
{
	m_texture->OnResetDevice();
}

void Font::OnLostDevice(void)
{
	m_texture->OnLostDevice();
}

void Font::OnDestroyDevice(void)
{
	m_texture->OnDestroyDevice();
	m_Device.Release();
}

void Font::CreateFontTexture(UINT Width, UINT Height)
{
	// ! DXUT dependency
	DXUTDeviceSettings settings = DXUTGetDeviceSettings();
	D3DPOOL Pool;
	if(settings.d3d9.DeviceType != D3DDEVTYPE_REF)
	{
		Pool = D3DPOOL_MANAGED;
	}
	else
	{
		Pool = D3DPOOL_SYSTEMMEM;
	}

	m_texture.reset();
	m_texture = Texture::CreateTexture(m_Device, Width, Height, 1, 0, D3DFMT_A8, Pool);
	m_textureDesc = m_texture->GetLevelDesc();
}

void Font::AssignTextureRect(const SIZE & size, RECT & outRect)
{
	if(!m_textureRectRoot->AssignRect(size, outRect))
	{
		_ASSERT(m_textureDesc.Width > 0 && m_textureDesc.Height > 0);

		CreateFontTexture(m_textureDesc.Width * 2, m_textureDesc.Height * 2);

		m_textureRectRoot = RectAssignmentNodePtr(new RectAssignmentNode(CRect(0, 0, m_textureDesc.Width, m_textureDesc.Height)));

		if(!m_textureRectRoot->AssignRect(size, outRect))
		{
			THROW_CUSEXCEPTION("m_textureRectRoot->AssignRect failed");
		}

		m_characterMap.clear();
	}
}

void Font::InsertCharacter(
	int character,
	int horiBearingX,
	int horiBearingY,
	int horiAdvance,
	const unsigned char * bmpBuffer,
	int bmpWidth,
	int bmpHeight,
	int bmpPitch)
{
	_ASSERT(m_characterMap.end() == m_characterMap.find(character));

	CharacterInfo info;

	// Add pixel gap around each font cell to avoid uv boundaries issue when Antialiasing
	AssignTextureRect(CSize(bmpWidth + FONT_PIXEL_GAP * 2, bmpHeight + FONT_PIXEL_GAP * 2), info.textureRect);

	::InflateRect(&info.textureRect, -FONT_PIXEL_GAP, -FONT_PIXEL_GAP);

	info.horiBearingX = horiBearingX;
	info.horiBearingY = horiBearingY;
	info.horiAdvance = horiAdvance;

	D3DLOCKED_RECT lr = m_texture->LockRect(info.textureRect);
	for(int y = 0; y < bmpHeight; y++)
	{
		memcpy(
			(unsigned char *)lr.pBits + y * lr.Pitch,
			bmpBuffer + y * bmpPitch,
			bmpWidth * sizeof(unsigned char));
	}
	m_texture->UnlockRect();

	m_characterMap.insert(std::make_pair(character, info));
}

void Font::LoadCharacter(int character)
{
	_ASSERT(m_characterMap.end() == m_characterMap.find(character));

	FT_UInt glyph_index = FT_Get_Char_Index(m_face, character);

	FT_Error err = FT_Load_Glyph(m_face, glyph_index, FT_LOAD_RENDER);
	if(err)
	{
		THROW_CUSEXCEPTION(str_printf("FT_Load_Glyph \"%c\" failed", character));
	}

	if(FT_PIXEL_MODE_GRAY != m_face->glyph->bitmap.pixel_mode)
	{
		THROW_CUSEXCEPTION("FT_PIXEL_MODE_GRAY != ft_face->glyph->bitmap.pixel_mode");
	}

	InsertCharacter(
		character,
		m_face->glyph->metrics.horiBearingX >> 6,
		m_face->glyph->metrics.horiBearingY >> 6,
		m_face->glyph->metrics.horiAdvance >> 6,
		m_face->glyph->bitmap.buffer,
		m_face->glyph->bitmap.width,
		m_face->glyph->bitmap.rows,
		m_face->glyph->bitmap.pitch);
}

const Font::CharacterInfo & Font::GetCharacterInfo(int character)
{
	CharacterMap::const_iterator char_info_iter = m_characterMap.find(character);
	if(m_characterMap.end() == char_info_iter)
	{
		LoadCharacter(character);
		char_info_iter = m_characterMap.find(character);
	}

	_ASSERT(m_characterMap.end() != char_info_iter);
	return (*char_info_iter).second;
}

Vector2 Font::CalculateStringExtent(LPCWSTR pString)
{
	Vector2 extent(0, m_LineHeight);
	wchar_t c;
	while((c = *pString++))
	{
		const CharacterInfo & info = GetCharacterInfo(c);
		extent.x += info.horiAdvance;
	}

	return extent;
}

Vector2 Font::CalculateAlignedPen(LPCWSTR pString, const my::Rectangle & rect, Align align)
{
	Vector2 pen;
	if(align & AlignLeft)
	{
		pen.x = rect.l;
	}
	else if(align & AlignCenter)
	{
		Vector2 extent = CalculateStringExtent(pString);
		pen.x = rect.l + (rect.r - rect.l - extent.x) * 0.5f;
	}
	else
	{
		Vector2 extent = CalculateStringExtent(pString);
		pen.x = rect.r - extent.x;
	}

	if(align & AlignTop)
	{
		pen.y = rect.t;
	}
	else if(align & AlignMiddle)
	{
		pen.y = rect.t + (rect.b - rect.t - m_LineHeight) * 0.5f;
	}
	else
	{
		pen.y = rect.b - m_LineHeight;
	}
	pen.y += m_LineHeight;

	return pen;
}

size_t Font::BuildStringVertices(
	CUSTOMVERTEX * pBuffer,
	size_t bufferSize,
	LPCWSTR pString,
	const my::Rectangle & rect,
	D3DCOLOR Color,
	Align align)
{
	Vector2 pen = CalculateAlignedPen(pString, rect, align);

	size_t i = 0;
	wchar_t c;
	while((c = *pString++) && pen.x < rect.r)
	{
		const CharacterInfo & info = GetCharacterInfo(c);

		int CharWidth = info.textureRect.right - info.textureRect.left;

		int CharHeight = info.textureRect.bottom - info.textureRect.top;

		// ! CalculateUVRect can be optimized
		size_t used = UIRender::BuildRectangleVertices(
			&pBuffer[i],
			bufferSize - i,
			Rectangle::LeftTop(pen.x + info.horiBearingX, pen.y - info.horiBearingY, CharWidth, CharHeight),
			Color,
			UIRender::CalculateUVRect(CSize(m_textureDesc.Width, m_textureDesc.Height), info.textureRect));

		if(0 == used)
		{
			break;
		}

		i += used;
		pen.x += info.horiAdvance;
	}

	return i;
}

void Font::DrawString(
	LPCWSTR pString,
	const my::Rectangle & rect,
	D3DCOLOR Color,
	Align align)
{
	UIRender::Begin(m_Device);

	V(m_Device->SetTexture(0, m_texture->m_ptr));

	// ! D3DFMT_A8
	V(m_Device->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_ALPHAREPLICATE));

	CUSTOMVERTEX vertex_list[1024];
	size_t numVerts = BuildStringVertices(vertex_list, _countof(vertex_list), pString, rect, Color, align);

	V(m_Device->SetFVF(D3DFVF_CUSTOMVERTEX));

	V(m_Device->DrawPrimitiveUP(D3DPT_TRIANGLELIST, numVerts / 3, vertex_list, sizeof(*vertex_list)));

	UIRender::End(m_Device);
}

void Font::DrawString(
	LPD3DXSPRITE pSprite,
	LPCWSTR pString,
	const my::Rectangle & rect,
	D3DCOLOR Color,
	Align align)
{
	Vector2 pen = CalculateAlignedPen(pString, rect, align);

	wchar_t c;
	while((c = *pString++) && pen.x < rect.r)
	{
		const CharacterInfo & info = GetCharacterInfo(c);

		V(m_Device->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_ALPHAREPLICATE));

		V(m_Device->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_POINT));
		V(m_Device->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_POINT));
		V(m_Device->SetSamplerState(0, D3DSAMP_MIPFILTER, D3DTEXF_NONE));

		V(pSprite->Draw(
			(IDirect3DTexture9 *)m_texture->m_ptr,
			&info.textureRect,
			(D3DXVECTOR3 *)&Vector3(0, 0, 0),
			(D3DXVECTOR3 *)&Vector3(pen.x + info.horiBearingX, pen.y - info.horiBearingY, 0),
			Color));

		pen.x += info.horiAdvance;
	}
}

float Font::CPtoX(LPCWSTR pString, int nCP)
{
	float x = 0;
	int i = 0;
	wchar_t c;
	for(; (c = *pString++) && i < nCP; i++)
	{
		const CharacterInfo & info = GetCharacterInfo(c);
		x += info.horiAdvance;
	}
	return x;
}

int Font::XtoCP(LPCWSTR pString, float _x)
{
	float x = 0;
	int i = 0;
	wchar_t c;
	for(; (c = *pString++); i++)
	{
		const CharacterInfo & info = GetCharacterInfo(c);
		if(x + info.horiAdvance > _x)
		{
			break;
		}
		x += info.horiAdvance;
	}
	return i;
}
