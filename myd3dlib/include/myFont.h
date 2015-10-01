#pragma once

#include <boost/shared_ptr.hpp>
#include "mySingleton.h"
#include "myTexture.h"
#include "myMath.h"
#include <boost/unordered_map.hpp>
#include <vector>

struct FT_LibraryRec_;

struct FT_FaceRec_;

namespace my
{
	class Sprite : public DeviceRelatedObject<ID3DXSprite>
	{
	public:
		Sprite(void)
		{
		}

		void Create(ID3DXSprite * ptr);

		void CreateSprite(LPDIRECT3DDEVICE9 pDevice);

		virtual void OnResetDevice(void);

		virtual void OnLostDevice(void);

		void Begin(DWORD Flags = D3DXSPRITE_ALPHABLEND);

		void Draw(Texture2DPtr texture, const CRect & SrcRect, const Vector3 & Center, const Vector3 & Position, D3DCOLOR Color);

		void End(void);

		CComPtr<IDirect3DDevice9> GetDevice(void);

		Matrix4 GetTransform(void);

		void SetTransform(const Matrix4 & Transform);

		void SetWorldViewLH(const Matrix4 & World, const Matrix4 & View);

		void SetWorldViewRH(const Matrix4 & World, const Matrix4 & View);
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

	public:
		RectAssignmentNode(const CRect & rect)
			: m_used(false)
			, m_rect(rect)
		{
		}

		bool AssignTopRect(const CSize & size, CRect & outRect);

		bool AssignLeftRect(const CSize & size, CRect & outRect);

		bool AssignRect(const CSize & size, CRect & outRect);
	};

	class UIRender;

	class Font : public DeviceRelatedObjectBase
	{
	public:
		class FontLibrary : public Singleton<FontLibrary>
		{
		public:
			FT_LibraryRec_ * m_Library;

			FontLibrary(void);

			~FontLibrary(void);
		};

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

		FT_FaceRec_ * m_face;

		HRESULT hr;

		CComPtr<IDirect3DDevice9> m_Device;

		const int FONT_PIXEL_GAP;

		boost::shared_ptr<std::vector<unsigned char> > m_cache;

		int m_Height;

		my::Vector2 m_Scale;

		int m_LineHeight;

		CharacterMap m_characterMap;

		Texture2DPtr m_Texture;

		D3DSURFACE_DESC m_textureDesc;

		RectAssignmentNodePtr m_textureRectRoot;

	public:
		Font(int font_pixel_gap = 1);

		virtual ~Font(void);

		const Vector2 & GetScale(void) const { return m_Scale; }

		void SetScale(const Vector2 & Scale);

		void Create(FT_FaceRec_ * face, int height, LPDIRECT3DDEVICE9 pDevice);

		void CreateFontFromFile(
			LPDIRECT3DDEVICE9 pDevice,
			LPCSTR pFilename,
			int height,
			long face_index = 0);

		void CreateFontFromFileInMemory(
			LPDIRECT3DDEVICE9 pDevice,
			const void * file_base,
			long file_size,
			int height,
			long face_index = 0);

		void CreateFontFromFileInCache(
			LPDIRECT3DDEVICE9 pDevice,
			boost::shared_ptr<std::vector<unsigned char> > cache_ptr,
			int height,
			long face_index = 0);

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

		void PushString(
			UIRender * ui_render,
			LPCWSTR pString,
			const Rectangle & rect,
			D3DCOLOR Color = D3DCOLOR_ARGB(255,255,255,255),
			Align align = AlignLeftTop);

		void DrawString(
			LPD3DXSPRITE pSprite,
			LPCWSTR pString,
			const Rectangle & rect,
			D3DCOLOR Color = D3DCOLOR_ARGB(255,255,255,255),
			Align align = AlignLeftTop);

		float CPtoX(LPCWSTR pString, int nCP);

		int XtoCP(LPCWSTR pString, float x);
	};

	typedef boost::shared_ptr<Font> FontPtr;
}
