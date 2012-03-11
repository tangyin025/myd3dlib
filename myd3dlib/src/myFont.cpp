
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

Font::Font(FT_Face face, float height, LPDIRECT3DDEVICE9 pDevice, unsigned short pixel_gap)
	: m_face(face)
	, m_Device(pDevice)
	, FONT_PIXEL_GAP(pixel_gap)
{
	_ASSERT(m_face);

	FT_Size_RequestRec_ req;
	req.type = FT_SIZE_REQUEST_TYPE_NOMINAL;
	req.width = (FT_Long)(height * 64.0f);
	req.height = (FT_Long)(height * 64.0f);
	req.horiResolution = 0;
	req.vertResolution = 0;
	FT_Error err = FT_Request_Size(m_face, &req);
	if(err)
	{
		THROW_CUSEXCEPTION("FT_Set_Pixel_Sizes failed");
	}

	m_LineHeight = m_face->size->metrics.height / 64.0f;

	m_maxAdvance = m_face->size->metrics.max_advance / 64.0f;

	m_texture = CreateFontTexture(pDevice, 256, 256);

	D3DSURFACE_DESC desc = m_texture->GetLevelDesc(0);
	m_textureRectRoot = RectAssignmentNodePtr(new RectAssignmentNode(CRect(0, 0, desc.Width, desc.Height)));
}

Font::~Font(void)
{
	FT_Done_Face(m_face);
}

TexturePtr Font::CreateFontTexture(LPDIRECT3DDEVICE9 pDevice, UINT Width, UINT Height)
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
	return Texture::CreateTexture(pDevice, Width, Height, 1, 0, D3DFMT_A8, Pool);
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
	CachePtr cache(new Cache(file_size));
	memcpy(&(*cache)[0], file_base, cache->size());

	FT_Face face;
	FT_Error err = FT_New_Memory_Face(ResourceMgr::getSingleton().m_library, &(*cache)[0], cache->size(), face_index, &face);
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

void Font::AssignTextureRect(const SIZE & size, RECT & outRect)
{
	D3DSURFACE_DESC desc = m_texture->GetLevelDesc(0);

	if(!m_textureRectRoot->AssignRect(size, outRect))
	{
		m_texture.reset();
		m_texture = CreateFontTexture(m_Device, desc.Width * 2, desc.Height * 2);

		desc = m_texture->GetLevelDesc(0);
		m_textureRectRoot = RectAssignmentNodePtr(new RectAssignmentNode(CRect(0, 0, desc.Width, desc.Height)));

		if(!m_textureRectRoot->AssignRect(size, outRect))
		{
			THROW_CUSEXCEPTION("m_textureRectRoot->AssignRect failed");
		}

		m_characterMap.clear();
	}
}

void Font::InsertCharacter(
	int character,
	float width,
	float height,
	float horiBearingX,
	float horiBearingY,
	float horiAdvance,
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

	info.width = width;
	info.height = height;
	info.horiBearingX = horiBearingX;
	info.horiBearingY = horiBearingY;
	info.horiAdvance = horiAdvance;
	info.uvRect.l = (float)info.textureRect.left / m_textureRectRoot->m_rect.right;
	info.uvRect.t = (float)info.textureRect.top / m_textureRectRoot->m_rect.bottom;
	info.uvRect.r = (float)info.textureRect.right / m_textureRectRoot->m_rect.right;
	info.uvRect.b = (float)info.textureRect.bottom / m_textureRectRoot->m_rect.bottom;

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
		m_face->glyph->metrics.width / 64.0f,
		m_face->glyph->metrics.height / 64.0f,
		m_face->glyph->metrics.horiBearingX / 64.0f,
		m_face->glyph->metrics.horiBearingY / 64.0f,
		m_face->glyph->metrics.horiAdvance / 64.0f,
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
	Vector2 extent = CalculateStringExtent(pString);

	Vector2 pen;
	if(align & AlignLeft)
	{
		pen.x = rect.l;
	}
	else if(align & AlignCenter)
	{
		pen.x = rect.l + (rect.r - rect.l - extent.x) * 0.5f;
	}
	else
	{
		pen.x = rect.r - extent.x;
	}
	if(align & AlignTop)
	{
		pen.y = rect.t;
	}
	else if(align & AlignMiddle)
	{
		pen.y = rect.t + (rect.b - rect.t - extent.y) * 0.5f;
	}
	else
	{
		pen.y = rect.b - extent.y;
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
	while((c = *pString++))
	{
		const CharacterInfo & info = GetCharacterInfo(c);

		size_t used = UIRender::BuildRectangleVertices(
			&pBuffer[i],
			bufferSize - i,
			my::Rectangle::LeftTop(pen.x + info.horiBearingX, pen.y - info.horiBearingY, info.width, info.height),
			info.uvRect,
			Color);

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
	while((c = *pString++))
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
