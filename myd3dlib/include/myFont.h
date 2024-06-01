// Copyright (c) 2011-2024 tangyin025
// License: MIT
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
	class Sprite : public D3DDeviceResource<ID3DXSprite>
	{
	public:
		Sprite(void)
		{
		}

		void Create(ID3DXSprite * ptr);

		void CreateSprite(void);

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

	class RectAssignmentNode : public CRect
	{
	public:
		bool m_used;

		RectAssignmentNodePtr m_lchild;

		RectAssignmentNodePtr m_rchild;

	public:
		RectAssignmentNode(int l, int t, int r, int b)
			: CRect(l, t, r, b)
			, m_used(false)
		{
		}

		bool AssignTopRect(const CSize & size, CRect & outRect);

		bool AssignLeftRect(const CSize & size, CRect & outRect);

		bool AssignRect(const CSize & size, CRect & outRect);
	};

	class FontLibrary : public SingletonInstance<FontLibrary>
	{
	public:
		FT_LibraryRec_ * m_Library;

		Vector2 m_Scale;

		typedef boost::signals2::signal<void(const Vector2 &)> ScaleEvent;

		ScaleEvent m_EventScaleChanged;

		FontLibrary(void);

		virtual ~FontLibrary(void);
	};

	class Font : public DeviceResourceBase
	{
	public:
		enum Align
		{
			AlignLeft			= 1,
			AlignCenter			= 2,
			AlignRight			= 4,
			AlignTop			= 8,
			AlignMiddle			= 16,
			AlignBottom			= 32,
			AlignMultiLine		= 64,
			AlignVertical		= 128,
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
			float vertBearingX;
			float vertBearingY;
			float vertAdvance;
			CRect textureRect;
		};

		typedef boost::unordered_map<unsigned long, CharacterInfo> CharacterMap;

		FT_FaceRec_ * m_face;

		HRESULT hr;

		const int FONT_PIXEL_GAP;

		boost::shared_ptr<std::vector<unsigned char> > m_cache;

		int m_Height;

		int m_LineHeight;

		CharacterMap m_characterMap;

		Texture2DPtr m_Texture;

		D3DSURFACE_DESC m_textureDesc;

		RectAssignmentNodePtr m_textureRectRoot;

	public:
		Font(int font_pixel_gap = 1);

		virtual ~Font(void);

		void SetScale(const Vector2 & Scale);

		Vector2 GetScale(void) const;

		void Create(FT_FaceRec_ * face, int height);

		void CreateFontFromFile(
			LPCSTR pFilename,
			int height,
			long face_index = 0);

		void CreateFontFromFileInMemory(
			const void * file_base,
			long file_size,
			int height,
			long face_index = 0);

		void CreateFontFromFileInCache(
			boost::shared_ptr<std::vector<unsigned char> > cache_ptr,
			int height,
			long face_index = 0);

		void OnResetDevice(void);

		void OnLostDevice(void);

		void OnDestroyDevice(void);

		void CreateFontTexture(UINT Width, UINT Height);

		void AssignTextureRect(const CSize & size, CRect & outRect);

		const CharacterInfo * InsertCharacter(
			unsigned long character,
			float width,
			float height,
			float horiBearingX,
			float horiBearingY,
			float horiAdvance,
			float vertBearingX,
			float vertBearingY,
			float vertAdvance,
			const unsigned char * bmpBuffer,
			int bmpWidth,
			int bmpHeight,
			int bmpPitch);

		const CharacterInfo * LoadCharacter(unsigned long character);

		const CharacterInfo * LoadCharacterOutline(unsigned long character, float outlineWidth);

		const CharacterInfo * GetCharacterInfo(unsigned long character);

		const CharacterInfo * GetCharacterOutlineInfo(unsigned long character, float outlineWidth);

		float CalculateStringWidth(LPCWSTR pString);

		float CalculateStringHeight(LPCWSTR pString);

		float CalculateStringLine(LPCWSTR pString);

		void DrawCharacter(LPD3DXSPRITE pSprite, float x, float y, const CharacterInfo * info, D3DCOLOR Color);

		void DrawString(
			const my::Rectangle & rect,
			LPCWSTR pString,
			Align align,
			const boost::function<const CharacterInfo * (wchar_t)> & get_character_info,
			const boost::function<void(float, float, const CharacterInfo *)> & draw_character);

		void DrawString(
			LPD3DXSPRITE pSprite,
			LPCWSTR pString,
			const Rectangle & rect,
			D3DCOLOR Color,
			Align align,
			D3DCOLOR OutlineColor,
			float OutlineWidth);

		float CPtoX(LPCWSTR pString, int nCP);

		int XtoCP(LPCWSTR pString, float x);
	};

	typedef boost::shared_ptr<Font> FontPtr;
}
