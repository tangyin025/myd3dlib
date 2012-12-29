#pragma once

#include "FileView.h"

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

class CImgRegionDoc;

class HistoryChange
{
public:
	CImgRegionDoc * m_pDoc;

	HistoryChange(CImgRegionDoc * pDoc)
		: m_pDoc(pDoc)
	{
	}

	virtual void Do(void) = 0;

	virtual void Undo(void) = 0;
};

typedef boost::shared_ptr<HistoryChange> HistoryChangePtr;

template <typename ValueType>
class HistoryChangeValue
	: public HistoryChange
{
public:
	ValueType m_oldValue;

	ValueType m_newValue;

	HistoryChangeValue(CImgRegionDoc * pDoc, const ValueType & oldValue, const ValueType & newValue)
		: HistoryChange(pDoc)
		, m_oldValue(oldValue)
		, m_newValue(newValue)
	{
	}
};

class HistoryChangeItemLocal
	: public HistoryChangeValue<CPoint>
{
public:
	std::wstring m_itemID;

	HistoryChangeItemLocal(CImgRegionDoc * pDoc, LPCTSTR itemID, const CPoint & oldValue, const CPoint & newValue)
		: HistoryChangeValue(pDoc, oldValue, newValue)
		, m_itemID(itemID)
	{
	}

	virtual void Do(void);

	virtual void Undo(void);
};

class History
	: public std::vector<HistoryChangePtr>
{
public:
	History(void)
	{
	}

	virtual void Do(void)
	{
		iterator hist_iter = begin();
		for(; hist_iter != end(); hist_iter++)
		{
			(*hist_iter)->Do();
		}
	}

	virtual void Undo(void)
	{
		iterator hist_iter = begin();
		for(; hist_iter != end(); hist_iter++)
		{
			(*hist_iter)->Undo();
		}
	}
};

typedef boost::shared_ptr<History> HistoryPtr;

class HistoryAddRegion
	: public History
{
public:
	CImgRegionDoc * m_pDoc;

	std::wstring m_itemID;

	CPoint m_Local;

	Gdiplus::Color m_Color;

	Gdiplus::Color m_FontColor;

	std::wstring m_parentID;

	HistoryAddRegion(CImgRegionDoc * pDoc, LPCTSTR itemID, const CPoint & Local, const Gdiplus::Color & Color, const Gdiplus::Color & FontColor, LPCTSTR parentID)
		: m_pDoc(pDoc)
		, m_itemID(itemID)
		, m_Local(Local)
		, m_Color(Color)
		, m_FontColor(FontColor)
		, m_parentID(parentID)
	{
	}

	virtual void Do(void);

	virtual void Undo(void);
};

class HistoryDelRegion
	: public History
{
public:
	CImgRegionDoc * m_pDoc;

	std::wstring m_itemID;

	std::wstring m_parentID;

	std::wstring m_beforeID;

	CMemFile m_DeleteNode;

	HistoryDelRegion(CImgRegionDoc * pDoc, LPCTSTR itemID)
		: m_pDoc(pDoc)
		, m_itemID(itemID)
	{
	}

	virtual void Do(void);

	virtual void Undo(void);
};

typedef std::vector<HistoryPtr> HistoryPtrList;

class CImgRegionDoc
	: public CDocument
{
public:
	CImgRegionTreeCtrl m_TreeCtrl;

	CSize m_ImageSizeTable[_countof(ZoomTable)];

	HistoryPtrList m_HistoryList;

	DWORD m_HistoryStep;

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

public:
	void SerializeRegionNode(CArchive & ar, CImgRegion * pReg);

	void SerializeRegionNodeSubTree(CArchive & ar, HTREEITEM hParent = TVI_ROOT);

	void UpdateImageSizeTable(const CSize & sizeRoot);

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

	afx_msg void OnEditUndo();

	afx_msg void OnUpdateEditUndo(CCmdUI *pCmdUI);

	afx_msg void OnEditRedo();

	afx_msg void OnUpdateEditRedo(CCmdUI *pCmdUI);
};
