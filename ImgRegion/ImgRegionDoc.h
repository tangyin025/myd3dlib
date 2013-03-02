#pragma once

#include "FileView.h"
#include "PropertiesWnd.h"

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

	CPoint m_Location;

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
		, m_Location(Local)
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
class HistoryChangeItemValue
	: public HistoryChange
{
public:
	std::wstring m_itemID;

	ValueType m_oldValue;

	ValueType m_newValue;

	CPropertiesWnd::Property m_property;

	HistoryChangeItemValue(CImgRegionDoc * pDoc, LPCTSTR itemID, const ValueType & oldValue, const ValueType & newValue)
		: HistoryChange(pDoc)
		, m_itemID(itemID)
		, m_oldValue(oldValue)
		, m_newValue(newValue)
	{
	}
};

class HistoryChangeItemLocation
	: public HistoryChangeItemValue<CPoint>
{
public:
	HistoryChangeItemLocation(CImgRegionDoc * pDoc, LPCTSTR itemID, const CPoint & oldValue, const CPoint & newValue)
		: HistoryChangeItemValue(pDoc, itemID, oldValue, newValue)
	{
	}

	virtual void Do(void);

	virtual void Undo(void);
};

class HistoryChangeItemSize
	: public HistoryChangeItemValue<CSize>
{
public:
	HistoryChangeItemSize(CImgRegionDoc * pDoc, LPCTSTR itemID, const CSize & oldValue, const CSize & newValue)
		: HistoryChangeItemValue(pDoc, itemID, oldValue, newValue)
	{
	}

	virtual void Do(void);

	virtual void Undo(void);
};

class HistoryChangeItemTextOff
	: public HistoryChangeItemValue<CPoint>
{
public:
	HistoryChangeItemTextOff(CImgRegionDoc * pDoc, LPCTSTR itemID, const CPoint & oldValue, const CPoint & newValue)
		: HistoryChangeItemValue(pDoc, itemID, oldValue, newValue)
	{
	}

	virtual void Do(void);

	virtual void Undo(void);
};

class History
{
public:
	History(void)
	{
	}

	virtual void Do(void) = 0;

	virtual void Undo(void) = 0;
};

typedef boost::shared_ptr<History> HistoryPtr;

typedef std::vector<HistoryPtr> HistoryPtrList;

class HistoryAddRegion
	: public History
{
public:
	CImgRegionDoc * m_pDoc;

	std::wstring m_itemID;

	std::wstring m_parentID;

	std::wstring m_beforeID;

	CMemFile m_NodeCache;

	DWORD m_OverideRegId;

	HistoryAddRegion(CImgRegionDoc * pDoc, LPCTSTR itemID, LPCTSTR parentID, LPCTSTR beforeID);

	virtual void Do(void);

	virtual void Undo(void);
};

typedef boost::shared_ptr<HistoryAddRegion> HistoryAddRegionPtr;

class HistoryDelRegion
	: public History
{
public:
	CImgRegionDoc * m_pDoc;

	std::wstring m_itemID;

	std::wstring m_parentID;

	std::wstring m_beforeID;

	CMemFile m_NodeCache;

	HistoryDelRegion(CImgRegionDoc * pDoc, LPCTSTR itemID)
		: m_pDoc(pDoc)
		, m_itemID(itemID)
	{
	}

	virtual void Do(void);

	virtual void Undo(void);
};

class HistoryMovRegion
	: public History
{
public:
	CImgRegionDoc * m_pDoc;

	std::wstring m_itemID;

	std::wstring m_oldParentID;

	std::wstring m_oldBeforeID;

	std::wstring m_newParentID;

	std::wstring m_newBeforeID;

	HistoryMovRegion(CImgRegionDoc * pDoc, LPCTSTR itemID, LPCTSTR newParentID, LPCTSTR newBeforeID)
		: m_pDoc(pDoc)
		, m_itemID(itemID)
		, m_newParentID(newParentID)
		, m_newBeforeID(newBeforeID)
	{
	}

	virtual void Do(void);

	virtual void Undo(void);
};

class HistoryModifyRegion
	: public History
	, public std::vector<HistoryChangePtr>
{
public:
	HistoryModifyRegion(void)
	{
	}

	virtual void Do(void);

	virtual void Undo(void);
};

typedef boost::shared_ptr<HistoryModifyRegion> HistoryModifyRegionPtr;

class CImgRegionDoc
	: public CDocument
{
public:
	CImgRegionTreeCtrl m_TreeCtrl;

	CImageList m_TreeImageList;

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

	void SerializeRegionNodeSubTree(CArchive & ar, HTREEITEM hParent = TVI_ROOT, BOOL bOverideName = FALSE);

	void UpdateImageSizeTable(const CSize & sizeRoot);

	void AddNewHistory(HistoryPtr hist);

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

	afx_msg void OnExportLua();
};
