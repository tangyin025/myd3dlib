#pragma once

#include "../myd3dbox/DragableTreeCtrl.h"
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

extern const TCHAR * TextAlignDesc[TextAlignCount];

class CImgRegion
{
public:
	BOOL m_Locked;

	CPoint m_Location;

	CSize m_Size;

	Gdiplus::Color m_Color;

	CString m_ImageStr;

	ImagePtr m_Image;

	CRect m_Rect;

	Vector4i m_Border;

	FontPtr2 m_Font;

	Gdiplus::Color m_FontColor;

	CString m_Text;

	DWORD m_TextAlign;

	BOOL m_TextWrap;

	CPoint m_TextOff;

	CImgRegion(void)
		: m_Locked(FALSE)
		, m_Location(100,100)
		, m_Size(100,100)
		, m_Color(255,255,255,255)
		, m_Border(0,0,0,0)
		, m_FontColor(255,255,255,255)
		, m_Text(_T("x:%d y:%d w:%d h:%d"))
		, m_TextAlign(TextAlignLeftTop)
		, m_TextWrap(FALSE)
		, m_TextOff(0,0)
	{
	}

	virtual void CreateProperties(CPropertiesWnd * pPropertiesWnd);

	virtual void UpdateProperties(CPropertiesWnd * pPropertiesWnd);

	virtual void Draw(Gdiplus::Graphics & grap);

	virtual void Serialize(CArchive& archive, int version);
};

typedef boost::shared_ptr<CImgRegion> CImgRegionPtr;

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
	UINT m_itemID;

	ValueType m_oldValue;

	ValueType m_newValue;

	CPropertiesWnd::Property m_property;

	HistoryChangeItemValue(CImgRegionDoc * pDoc, UINT itemID, const ValueType & oldValue, const ValueType & newValue)
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
	HistoryChangeItemLocation(CImgRegionDoc * pDoc, UINT itemID, const CPoint & oldValue, const CPoint & newValue)
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
	HistoryChangeItemSize(CImgRegionDoc * pDoc, UINT itemID, const CSize & oldValue, const CSize & newValue)
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
	HistoryChangeItemTextOff(CImgRegionDoc * pDoc, UINT itemID, const CPoint & oldValue, const CPoint & newValue)
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

	UINT m_itemID;

	std::basic_string<TCHAR> m_strItem;

	UINT m_parentID;

	UINT m_beforeID;

	CMemFile m_NodeCache;

	DWORD m_OverideRegId;

	HistoryAddRegion(CImgRegionDoc * pDoc, UINT itemID, LPCTSTR lpszItem, UINT parentID, UINT beforeID);

	virtual void Do(void);

	virtual void Undo(void);
};

typedef boost::shared_ptr<HistoryAddRegion> HistoryAddRegionPtr;

class HistoryDelRegion
	: public History
{
public:
	CImgRegionDoc * m_pDoc;

	UINT m_itemID;

	std::basic_string<TCHAR> m_strItem;

	UINT m_parentID;

	UINT m_beforeID;

	CMemFile m_NodeCache;

	HistoryDelRegion(CImgRegionDoc * pDoc, UINT itemID)
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

	UINT m_itemID;

	UINT m_oldParentID;

	UINT m_oldBeforeID;

	UINT m_newParentID;

	UINT m_newBeforeID;

	HistoryMovRegion(CImgRegionDoc * pDoc, UINT itemID, UINT newParentID, UINT newBeforeID)
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
	typedef boost::unordered_map<UINT, HTREEITEM> HTREEITEMMap;

	HTREEITEMMap m_ItemMap;

	typedef std::pair<UINT, CImgRegionPtr> ItemDataType;

	CDragableTreeCtrl m_TreeCtrl;

	CImageList m_TreeImageList;

	HistoryPtrList m_HistoryList;

	DWORD m_HistoryStep;

	DWORD m_NextRegId;

	CSize m_Size;

	Gdiplus::Color m_Color;

	CString m_ImageStr;

	ImagePtr m_Image;

	CString m_strProjectDir;

	CString m_strLuaPath;

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

public:
	BOOL CanItemMove(HTREEITEM hMoveItem, HTREEITEM hParent, HTREEITEM hInsertAfter);

	HTREEITEM InsertItem(UINT id, const std::basic_string<TCHAR> & strItem, CImgRegionPtr reg_ptr, HTREEITEM hParent = TVI_ROOT, HTREEITEM hInsertAfter = TVI_LAST);

	UINT GetItemId(HTREEITEM hItem);

	CImgRegionPtr GetItemNode(HTREEITEM hItem);

	void DeleteTreeItem(HTREEITEM hItem);

	HTREEITEM MoveTreeItem(HTREEITEM hParent, HTREEITEM hInsertAfter, HTREEITEM hOtherItem);

	void SerializeSubTreeNode(CArchive & ar, int version, HTREEITEM hParent = TVI_ROOT, BOOL bOverideName = FALSE);

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
