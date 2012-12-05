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
	CRect m_rc;

	Gdiplus::Color m_color;

	ImagePtr m_image;

	Vector4i m_border;

	FontPtr2 m_font;

	CImgRegion(const CRect & rc, const Gdiplus::Color & color)
		: m_rc(rc)
		, m_color(color)
		, m_border(0,0,0,0)
	{
	}
};

class CImgRegionDoc : public CDocument
{
public:
	CTreeCtrl m_TreeCtrl;

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
