#pragma once

typedef boost::shared_ptr<Gdiplus::Image> ImagePtr;

typedef boost::shared_ptr<Gdiplus::Font> FontPtr2;

class Vector4i
{
public:
	int x, y, z, w;

	Vector4i(int _x, int _y, int _z, int _w)
		: x(_x), y(_y), z(_z), w(_w)
	{
	}

	Vector4i(void)
	{
	}
};

class CImgRegion
{
public:
	CPoint m_Local;

	CSize m_Size;

	Gdiplus::Color m_Color;

	ImagePtr m_Image;

	Vector4i m_Border;

	FontPtr2 m_Font;

	CImgRegion(const CPoint & Local, const CSize & Size, const Gdiplus::Color & Color, const Vector4i & Border = Vector4i(0,0,0,0))
		: m_Local(Local)
		, m_Size(Size)
		, m_Color(Color)
		, m_Border(Border)
	{
	}
};

class CImgRegionDoc : public CDocument
{
public:
	CTreeCtrl m_TreeCtrl;

	CSize m_ImageSize;

	Gdiplus::Color m_BkColor;

public:
	DECLARE_DYNCREATE(CImgRegionDoc)

	CImgRegionDoc(void);

	BOOL LocalToRoot(HTREEITEM hItem, const CPoint & ptLocal, CPoint & ptResult);

	BOOL CreateTreeCtrl(void);

	void DestroyTreeCtrl(void);

	void DeleteItemTreeData(HTREEITEM hItem);

	HTREEITEM GetPointedRegionNode(HTREEITEM hItem, const CPoint & ptLocal);

	DECLARE_MESSAGE_MAP()

	virtual BOOL OnNewDocument(void);

	virtual void OnCloseDocument();
};
