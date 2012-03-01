
#pragma once

namespace my
{
	class Sprite;

	typedef boost::shared_ptr<Sprite> SpritePtr;

	class Sprite : public DeviceRelatedObject<ID3DXSprite>
	{
	protected:
		Sprite(ID3DXSprite * ptr)
			: DeviceRelatedObject(ptr)
		{
		}

		virtual void OnResetDevice(void)
		{
			V(m_ptr->OnResetDevice());
		}

		virtual void OnLostDevice(void)
		{
			V(m_ptr->OnLostDevice());
		}

	public:
		static SpritePtr CreateSprite(LPDIRECT3DDEVICE9 pDevice);

		void Begin(DWORD Flags = D3DXSPRITE_ALPHABLEND)
		{
			V(m_ptr->Begin(Flags));
		}

		void Draw(TexturePtr texture, const RECT & SrcRect, const Vector3 & Center, const Vector3 & Position, D3DCOLOR Color)
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

	class RectAssignmentNode;

	typedef boost::shared_ptr<RectAssignmentNode> RectAssignmentNodePtr;

	class RectAssignmentNode
	{
	protected:
		bool m_used;

		RECT m_rect;

		RectAssignmentNodePtr m_lchild;

		RectAssignmentNodePtr m_rchild;

		bool AssignTopRect(const SIZE & size, RECT & outRect);

		bool AssignLeftRect(const SIZE & size, RECT & outRect);

	public:
		RectAssignmentNode(const RECT & rect)
			: m_used(false)
			, m_rect(rect)
		{
		}

		bool AssignRect(const SIZE & size, RECT & outRect);
	};

	class FontMgr
		: public Singleton<FontMgr>
	{
	public:
		FT_Library m_library;

	public:
		FontMgr(void);

		~FontMgr(void);
	};

	class Font;

	typedef boost::shared_ptr<Font> FontPtr;

	class Font : public DeviceRelatedObjectBase
	{
	public:
		enum Align
		{
			alignLeftTop,
		};

		struct CharacterInfo
		{
			RECT textureRect;

			int horiAdvance;

			int horiBearingX;

			int horiBearingY;
		};

		typedef std::map<int, CharacterInfo> CharacterMap;

	protected:
		HRESULT hr;

		FT_Face m_face;

		CComPtr<IDirect3DDevice9> m_Device;

		CachePtr m_cache;

		int m_lineHeight;

		int m_maxAdvance;

		CharacterMap m_characterMap;

		TexturePtr m_texture;

		RectAssignmentNodePtr m_textureRectRoot;

		Font(FT_Face face, int height, LPDIRECT3DDEVICE9 pDevice);

	public:
		virtual ~Font(void);

		static FontPtr CreateFontFromFile(
			LPDIRECT3DDEVICE9 pDevice,
			LPCSTR pFilename,
			int height,
			FT_Long face_index = 0);

		static FontPtr CreateFontFromFileInMemory(
			LPDIRECT3DDEVICE9 pDevice,
			const void * file_base,
			long file_size,
			int height,
			long face_index = 0);

		static TexturePtr CreateFontTexture(LPDIRECT3DDEVICE9 pDevice, UINT Width, UINT Height);

		virtual void OnResetDevice(void);

		virtual void OnLostDevice(void);

		virtual void OnDestroyDevice(void);

		void AssignTextureRect(const SIZE & size, RECT & outRect);

		void InsertCharacter(
			int character,
			int horiAdvance,
			int horiBearingX,
			int horiBearingY,
			const unsigned char * bmpBuffer,
			int bmpWidth,
			int bmpHeight,
			int bmpPitch);

		void LoadCharacter(int character);

		const CharacterInfo & GetCharacterInfo(int character);

		void DrawString(
			LPD3DXSPRITE pSprite,
			LPCWSTR pString,
			const Rectangle & rect,
			D3DCOLOR Color = D3DCOLOR_ARGB(255, 255, 255, 255),
			Align align = alignLeftTop);
	};
}
