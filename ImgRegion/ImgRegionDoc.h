#pragma once

#include "FileView.h"

typedef boost::shared_ptr<Gdiplus::Image> ImagePtr;

typedef boost::shared_ptr<Gdiplus::Font> FontPtr2;

#define TVN_DRAGCHANGED (TVN_LAST + 1)

struct NMTREEVIEWDRAG
{
	NMHDR hdr;
	HTREEITEM hDragItem;
	HTREEITEM hDragTagParent;
	HTREEITEM hDragTagFront;
};

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

class CImgRegion
{
public:
	CPoint m_Local;

	CSize m_Size;

	Gdiplus::Color m_Color;

	ImagePtr m_Image;

	CString m_ImageStr;

	Vector4i m_Border;

	FontPtr2 m_Font;

	Gdiplus::Color m_FontColor;

	CString m_Text;

	CImgRegion(const CPoint & Local, const CSize & Size, const Gdiplus::Color & Color, const Vector4i & Border = Vector4i(0,0,0,0))
		: m_Local(Local)
		, m_Size(Size)
		, m_Color(Color)
		, m_Border(Border)
		, m_FontColor(255,0,0,255)
		, m_Text(_T("x:%d y:%d w:%d h:%d"))
	{
	}
};

class CImgRegionTreeCtrl : public CTreeCtrl
{
protected:
	BOOL m_bDrag;

	HTREEITEM m_hDragItem;

	HTREEITEM m_hDragTagParent;

	HTREEITEM m_hDragTagFront;

	DECLARE_DYNAMIC(CImgRegionTreeCtrl)

public:
	CImgRegionTreeCtrl(void);

	DECLARE_MESSAGE_MAP()

	afx_msg void OnTvnBegindrag(NMHDR *pNMHDR, LRESULT *pResult);

	afx_msg void OnMouseMove(UINT nFlags, CPoint point);

	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);

public:
	BOOL FindTreeChildItem(HTREEITEM hParent, HTREEITEM hChild);

	HTREEITEM MoveTreeItem(HTREEITEM hParent, HTREEITEM hInsertAfter, HTREEITEM hOtherItem);

	template <class DataType>
	void DeleteTreeItem(HTREEITEM hItem, BOOL bDeleteData = FALSE)
	{
		if(bDeleteData)
		{
			delete (DataType *)GetItemData(hItem);

			HTREEITEM hNextChild = NULL;
			for(HTREEITEM hChild = GetChildItem(hItem); NULL != hChild; hChild = hNextChild)
			{
				hNextChild = GetNextSiblingItem(hChild);

				DeleteTreeItem<DataType>(hChild, bDeleteData);
			}
		}

		DeleteItem(hItem);
	}

	HTREEITEM GetSafeParentItem(HTREEITEM hItem);

	HTREEITEM GetSafePreSiblingItem(HTREEITEM hItem);
};

class CImgRegionDoc
	: public CDocument
	, public CImgRegion
{
public:
	CImgRegionTreeCtrl m_TreeCtrl;

	CString m_CurrentDir;

	typedef std::tr1::unordered_map<std::wstring, ImagePtr, boost::hash<std::wstring> > ImagePtrMap;

	ImagePtrMap m_ImageMap;

	typedef std::tr1::unordered_map<std::wstring, FontPtr2, boost::hash<std::wstring> > FontPtr2Map;

	FontPtr2Map m_FontMap;

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

	void SerializeRegionNode(CArchive & ar, HTREEITEM hParent = TVI_ROOT);

public:
	ImagePtr GetImage(CString strImg);

	FontPtr2 GetFont(const CString & strFamily, float fSize);

	const CString & GetCurrentDir(void) const;

	CString GetRelativePath(const CString & strPath) const;

	CString GetFullPath(const CString & strPath) const;
};
