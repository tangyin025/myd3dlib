
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

FontMgr::DrivedClassPtr Singleton<FontMgr>::s_ptr;

FontMgr::FontMgr(void)
{
	FT_Error err = FT_Init_FreeType(&m_library);
	if(err)
	{
		THROW_CUSEXCEPTION("FT_Init_FreeType failed");
	}
}

FontMgr::~FontMgr(void)
{
	FT_Error err = FT_Done_FreeType(m_library);
}

Font::Font(FT_Face face, int height, LPDIRECT3DDEVICE9 pDevice)
	: m_face(face)
	, m_Device(pDevice)
{
	_ASSERT(m_face);

	FT_Error err = FT_Set_Pixel_Sizes(m_face, height, height);
	if(err)
	{
		THROW_CUSEXCEPTION("FT_Set_Pixel_Sizes failed");
	}

	m_lineHeight = m_face->size->metrics.height >> 6;

	m_maxAdvance = m_face->size->metrics.max_advance >> 6;

	m_texture = CreateFontTexture(pDevice, m_maxAdvance, m_lineHeight);

	m_textureRectRoot = RectAssignmentNodePtr(new RectAssignmentNode(CRect(0, 0, m_maxAdvance, m_lineHeight)));
}

Font::~Font(void)
{
	FT_Done_Face(m_face);
}

TexturePtr Font::CreateFontTexture(LPDIRECT3DDEVICE9 pDevice, UINT Width, UINT Height)
{
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
	int height,
	FT_Long face_index /*= 0*/)
{
	FT_Face face;
	FT_Error err = FT_New_Face(FontMgr::getSingleton().m_library, pFilename, face_index, &face);
	if(err)
	{
		THROW_CUSEXCEPTION("FT_New_Face failed");
	}

	FontPtr font(new Font(face, height, pDevice));
	return font;
}

FontPtr Font::CreateFontFromFileInMemory(
	LPDIRECT3DDEVICE9 pDevice,
	const void * file_base,
	long file_size,
	int height,
	long face_index /*= 0*/)
{
	CachePtr cache(new Cache(file_size));
	memcpy(&(*cache)[0], file_base, cache->size());

	FT_Face face;
	FT_Error err = FT_New_Memory_Face(FontMgr::getSingleton().m_library, &(*cache)[0], cache->size(), face_index, &face);
	if(err)
	{
		THROW_CUSEXCEPTION("FT_New_Memory_Face failed");
	}

	FontPtr font(new Font(face, height, pDevice));
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
		m_textureRectRoot = RectAssignmentNodePtr(new RectAssignmentNode(CRect(0, 0, desc.Width * 2, desc.Height * 2)));

		if(!m_textureRectRoot->AssignRect(size, outRect))
		{
			THROW_CUSEXCEPTION("m_textureRectRoot->AssignRect failed");
		}

		m_characterMap.clear();
	}
}

void Font::InsertCharacter(
	int character,
	int horiAdvance,
	int horiBearingX,
	int horiBearingY,
	const unsigned char * bmpBuffer,
	int bmpWidth,
	int bmpHeight,
	int bmpPitch)
{
	_ASSERT(m_characterMap.end() == m_characterMap.find(character));

	CharacterInfo cm;
	AssignTextureRect(CSize(bmpWidth, bmpHeight), cm.textureRect);
	cm.horiAdvance = horiAdvance;
	cm.horiBearingX = horiBearingX;
	cm.horiBearingY = horiBearingY;

	D3DLOCKED_RECT lr = m_texture->LockRect(cm.textureRect);
	for(int y = 0; y < bmpHeight; y++)
	{
		memcpy(
			(unsigned char *)lr.pBits + y * lr.Pitch,
			bmpBuffer + y * bmpPitch,
			bmpWidth * sizeof(unsigned char));
	}
	m_texture->UnlockRect();

	m_characterMap.insert(std::make_pair(character, cm));
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
		m_face->glyph->metrics.horiAdvance >> 6,
		m_face->glyph->metrics.horiBearingX >> 6,
		m_face->glyph->metrics.horiBearingY >> 6,
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

void Font::DrawString(
	LPD3DXSPRITE pSprite,
	LPCWSTR pString,
	const my::Rectangle & rect,
	D3DCOLOR Color /*= D3DCOLOR_ARGB(255, 255, 255, 255)*/,
	Align align /*= alignLeftTop*/)
{
	Vector3 pen(rect.l, rect.t + m_lineHeight, 0);
	wchar_t c;
	while((c = *pString++))
	{
		switch(c)
		{
		case L'\r':
			break;

		case L'\n':
			pen.x = rect.l;
			pen.y += m_lineHeight;
			break;

		default:
			{
				const CharacterInfo & info = GetCharacterInfo(c);
				V(m_Device->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_ALPHAREPLICATE));
				V(pSprite->Draw(
					(IDirect3DTexture9 *)m_texture->m_ptr, &info.textureRect, (D3DXVECTOR3 *)&Vector3(0, 0, 0), (D3DXVECTOR3 *)&Vector3(pen.x + info.horiBearingX, pen.y - info.horiBearingY, 0), Color));
				pen.x += info.horiAdvance;
			}
			break;
		}
	}
}
