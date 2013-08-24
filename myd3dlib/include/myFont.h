#pragma once

#include <boost/shared_ptr.hpp>
#include "mySingleton.h"
#include <d3dx9.h>
#include "myTexture.h"
#include <atltypes.h>
#include "myMath.h"
#include <boost/unordered_map.hpp>
#include <ft2build.h>
#include FT_FREETYPE_H
#include <vector>

namespace my
{
	class Sprite : public DeviceRelatedObject<ID3DXSprite>
	{
	public:
		Sprite(void)
		{
		}

		void Create(ID3DXSprite * ptr)
		{
			_ASSERT(!m_ptr);

			m_ptr = ptr;
		}

		void CreateSprite(LPDIRECT3DDEVICE9 pDevice);

		virtual void OnResetDevice(void)
		{
			V(m_ptr->OnResetDevice());
		}

		virtual void OnLostDevice(void)
		{
			V(m_ptr->OnLostDevice());
		}

		void Begin(DWORD Flags = D3DXSPRITE_ALPHABLEND)
		{
			V(m_ptr->Begin(Flags));
		}

		void Draw(TexturePtr texture, const CRect & SrcRect, const Vector3 & Center, const Vector3 & Position, D3DCOLOR Color)
		{
			V(m_ptr->Draw(static_cast<IDirect3DTexture9 *>(texture->m_ptr), &SrcRect, (D3DXVECTOR3 *)&Center, (D3DXVECTOR3 *)&Position, Color));
		}

		void End(void)
		{
			V(m_ptr->End());
		}

		CComPtr<IDirect3DDevice9> GetDevice(void)
		{
			CComPtr<IDirect3DDevice9> ret;
			V(m_ptr->GetDevice(&ret));
			return ret;
		}

		Matrix4 GetTransform(void)
		{
			Matrix4 ret;
			V(m_ptr->GetTransform((D3DXMATRIX *)&ret));
			return ret;
		}

		void SetTransform(const Matrix4 & Transform)
		{
			V(m_ptr->SetTransform((D3DXMATRIX *)&Transform));
		}

		void SetWorldViewLH(const Matrix4 & World, const Matrix4 & View)
		{
			V(m_ptr->SetWorldViewLH((D3DXMATRIX *)&World, (D3DXMATRIX *)&View));
		}

		void SetWorldViewRH(const Matrix4 & World, const Matrix4 & View)
		{
			V(m_ptr->SetWorldViewRH((D3DXMATRIX *)&World, (D3DXMATRIX *)&View));
		}
	};

	typedef boost::shared_ptr<Sprite> SpritePtr;

	class RectAssignmentNode;

	typedef boost::shared_ptr<RectAssignmentNode> RectAssignmentNodePtr;

	class RectAssignmentNode
	{
	public:
		bool m_used;

		CRect m_rect;

		RectAssignmentNodePtr m_lchild;

		RectAssignmentNodePtr m_rchild;

		bool AssignTopRect(const CSize & size, CRect & outRect);

		bool AssignLeftRect(const CSize & size, CRect & outRect);

	public:
		RectAssignmentNode(const CRect & rect)
			: m_used(false)
			, m_rect(rect)
		{
		}

		bool AssignRect(const CSize & size, CRect & outRect);
	};

	class FontLibrary : public Singleton<FontLibrary>
	{
	public:
		FT_Library m_Library;

		FontLibrary(void)
		{
			FT_Error err = FT_Init_FreeType(&m_Library);
			if(err)
			{
				THROW_CUSEXCEPTION(_T("FT_Init_FreeType failed"));
			}
		}

		virtual ~FontLibrary(void)
		{
			FT_Done_FreeType(m_Library);
		}
	};

	class UIRender;

	class Font : public DeviceRelatedObjectBase
	{
	public:
		enum Align
		{
			AlignLeft			= 0x000001,
			AlignCenter			= 0x000010,
			AlignRight			= 0x000100,
			AlignTop			= 0x001000,
			AlignMiddle			= 0x010000,
			AlignBottom			= 0x100000,
			AlignLeftTop		= AlignLeft | AlignTop,
			AlignCenterTop		= AlignCenter | AlignTop,
			AlignRightTop		= AlignRight | AlignTop,
			AlignLeftMiddle		= AlignLeft | AlignMiddle,
			AlignCenterMiddle	= AlignCenter | AlignMiddle,
			AlignRightMiddle	= AlignRight | AlignMiddle,
			AlignLeftBottom		= AlignLeft | AlignBottom,
			AlignCenterBottom	= AlignCenter | AlignBottom,
			AlignRightBottom	= AlignRight | AlignBottom,
		};

		struct CharacterInfo
		{
			float width;
			float height;
			float horiBearingX;
			float horiBearingY;
			float horiAdvance;
			CRect textureRect;
		};

		typedef boost::unordered_map<int, CharacterInfo> CharacterMap;

		FT_Face m_face;

		HRESULT hr;

		CComPtr<IDirect3DDevice9> m_Device;

		const int FONT_PIXEL_GAP;

		boost::shared_ptr<std::vector<unsigned char> > m_cache;

		int m_Height;

		my::Vector2 m_Scale;

		float m_LineHeight;

		CharacterMap m_characterMap;

		Texture m_Texture;

		D3DSURFACE_DESC m_textureDesc;

		RectAssignmentNodePtr m_textureRectRoot;

	public:
		Font(int font_pixel_gap = 1)
			: m_face(NULL)
			, FONT_PIXEL_GAP(font_pixel_gap)
			, m_Scale(0,0)
		{
		}

		virtual ~Font(void);

		const Vector2 & GetScale(void) const { return m_Scale; }

		void SetScale(const Vector2 & Scale);

		void Create(FT_Face face, int height, LPDIRECT3DDEVICE9 pDevice);

		void CreateFontFromFile(
			LPDIRECT3DDEVICE9 pDevice,
			LPCSTR pFilename,
			int height,
			FT_Long face_index = 0);

		void CreateFontFromFileInMemory(
			LPDIRECT3DDEVICE9 pDevice,
			const void * file_base,
			long file_size,
			int height,
			FT_Long face_index = 0);

		void CreateFontFromFileInCache(
			LPDIRECT3DDEVICE9 pDevice,
			boost::shared_ptr<std::vector<unsigned char> > cache_ptr,
			int height,
			FT_Long face_index = 0);

		void OnResetDevice(void);

		void OnLostDevice(void);

		void OnDestroyDevice(void);

		void CreateFontTexture(UINT Width, UINT Height);

		void AssignTextureRect(const CSize & size, CRect & outRect);

		void InsertCharacter(
			int character,
			float width,
			float height,
			float horiBearingX,
			float horiBearingY,
			float horiAdvance,
			const unsigned char * bmpBuffer,
			int bmpWidth,
			int bmpHeight,
			int bmpPitch);

		void LoadCharacter(int character);

		const CharacterInfo & GetCharacterInfo(int character);

		Vector2 CalculateStringExtent(LPCWSTR pString);

		Vector2 CalculateAlignedPen(LPCWSTR pString, const Rectangle & rect, Align align);

		void DrawString(
			UIRender * ui_render,
			LPCWSTR pString,
			const Rectangle & rect,
			D3DCOLOR Color = D3DCOLOR_ARGB(255, 255, 255, 255),
			Align align = AlignLeftTop);

		void DrawString(
			LPD3DXSPRITE pSprite,
			LPCWSTR pString,
			const Rectangle & rect,
			D3DCOLOR Color = D3DCOLOR_ARGB(255, 255, 255, 255),
			Align align = AlignLeftTop);

		float CPtoX(LPCWSTR pString, int nCP);

		int XtoCP(LPCWSTR pString, float x);
	};

	typedef boost::shared_ptr<Font> FontPtr;
}
