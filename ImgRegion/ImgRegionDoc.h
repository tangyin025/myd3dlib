#pragma once

#include "FileView.h"
#include "MainApp.h"

static const float ZoomTable[] = {
	32, 16, 12, 8, 7, 6, 5, 4, 3, 2, 1, 2.0f/3, 1.0f/2, 1.0f/3, 1.0f/4, 1.0f/6, 1.0f/8, 1.0f/12, 1.0f/16, 1.0f/20, 1.0f/25, 3.0f/100, 2.0f/100, 1.5f/100, 1.0f/100, 0.7f/100 };

class Vector4i
{
public:
	LONG x, y, z, w;

	Vector4i(int _x, int _y, int _z, int _w)
		: x(_x), y(_y), z(_z), w(_w)
	{
	}

	Vector4i(void)
	{
	}
};

enum TextAlign
{
	TextAlignLeftTop = 0,
	TextAlignCenterTop,
	TextAlignRightTop,
	TextAlignLeftMiddle,
	TextAlignCenterMiddle,
	TextAlignRightMiddle,
	TextAlignLeftBottom,
	TextAlignCenterBottom,
	TextAlignRightBottom,
	TextAlignCount
};

class CImgRegion
{
public:
	BOOL m_Locked;

	CPoint m_Local;

	CSize m_Size;

	Gdiplus::Color m_Color;

	CString m_ImageStr;

	ImagePtr m_Image;

	Vector4i m_Border;

	FontPtr2 m_Font;

	Gdiplus::Color m_FontColor;

	CString m_Text;

	DWORD m_TextAlign;

	BOOL m_TextWrap;

	CPoint m_TextOff;

	CImgRegion(const CPoint & Local, const CSize & Size, const Gdiplus::Color & Color = Gdiplus::Color::White, const Vector4i & Border = Vector4i(0,0,0,0))
		: m_Locked(FALSE)
		, m_Local(Local)
		, m_Size(Size)
		, m_Color(Color)
		, m_Border(Border)
		, m_FontColor(255,0,0,0)
		, m_Text(_T("x:%d y:%d w:%d h:%d"))
		, m_TextAlign(TextAlignLeftTop)
		, m_TextWrap(FALSE)
		, m_TextOff(0,0)
	{
	}
};

class CImgRegionDoc
	: public CDocument
{
public:
	CImgRegionTreeCtrl m_TreeCtrl;

	CSize m_ImageSizeTable[_countof(ZoomTable)];

	DWORD m_NextRegId;

	CSize m_Size;

	Gdiplus::Color m_Color;

	CString m_ImageStr;

	ImagePtr m_Image;

public:
	DECLARE_DYNCREATE(CImgRegionDoc)

	CImgRegionDoc(void);

	CPoint LocalToRoot(HTREEITEM hItem, const CPoint & ptLocal);

	CPoint RootToLocal(HTREEITEM hItem, const CPoint & ptRoot);

	BOOL CreateTreeCtrl(void);

	void DestroyTreeCtrl(void);

	HTREEITEM GetPointedRegionNode(HTREEITEM hItem, const CPoint & ptLocal);

	DECLARE_MESSAGE_MAP()

	virtual BOOL OnNewDocument(void);

	virtual BOOL OnOpenDocument(LPCTSTR lpszPathName);

	virtual BOOL OnSaveDocument(LPCTSTR lpszPathName);

	virtual void OnCloseDocument();

	virtual void Serialize(CArchive& ar);

	int GetChildCount(HTREEITEM hItem);

	void SerializeRegionNode(CArchive & ar, CImgRegion * pReg);

	void SerializeRegionNodeTree(CArchive & ar, HTREEITEM hParent = TVI_ROOT);

	void UpdateImageSizeTable(const CSize & sizeRoot);
public:
	afx_msg void OnAddRegion();

	afx_msg void OnUpdateAddRegion(CCmdUI *pCmdUI);

	afx_msg void OnDelRegion();

	afx_msg void OnUpdateDelRegion(CCmdUI *pCmdUI);

	afx_msg void OnExportImg();

	afx_msg void OnFileProperty();

	afx_msg void OnEditCopy();

	afx_msg void OnUpdateEditCopy(CCmdUI *pCmdUI);

	afx_msg void OnEditPaste();

	afx_msg void OnUpdateEditPaste(CCmdUI *pCmdUI);
};
