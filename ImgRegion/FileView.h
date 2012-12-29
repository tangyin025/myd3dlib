
#pragma once

class CImgRegionDoc;

class CImgRegionTreeCtrl : public CTreeCtrl
{
protected:
	BOOL m_bDrag;

	HTREEITEM m_hDragItem;

	CPoint m_LastDragPos;

	enum DropType
	{
		DropTypeNone = 0,
		DropTypeFront,
		DropTypeChild,
		DropTypeBack,
	};

	DropType m_DragDropType;

	enum TimerEvent
	{
		TimerEventUnknown = 0,
		TimerEventScrollUp,
		TimerEventScrollDown,
	};

	DECLARE_DYNAMIC(CImgRegionTreeCtrl)

	typedef std::tr1::unordered_map<std::wstring, HTREEITEM, boost::hash<std::wstring> > HTREEITEMMap;

	HTREEITEMMap m_ItemMap;

public:
	CImgRegionTreeCtrl(void);

	DECLARE_MESSAGE_MAP()

	afx_msg void OnTvnBegindrag(NMHDR *pNMHDR, LRESULT *pResult);

	afx_msg void OnMouseMove(UINT nFlags, CPoint point);

	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);

public:
	HTREEITEM InsertItem(LPCTSTR lpszItem, HTREEITEM hParent = TVI_ROOT , HTREEITEM hInsertAfter = TVI_LAST);

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

		std::wstring key(GetItemText(hItem));
		ASSERT(m_ItemMap.find(key) != m_ItemMap.end());
		DeleteItem(hItem);
		m_ItemMap.erase(key);
	}

	HTREEITEM GetSafeParentItem(HTREEITEM hItem);

	HTREEITEM GetSafePreSiblingItem(HTREEITEM hItem);

	afx_msg void OnTimer(UINT_PTR nIDEvent);
};

class CFileView : public CDockablePane
{
public:
	CImgRegionDoc * m_pDoc;

	BOOL m_bIsLayoutInvalid;

public:
	CFileView(void);

	void AdjustLayout(void);

	void OnChangeVisualStyle(void);

	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);

	afx_msg void OnSize(UINT nType, int cx, int cy);

	afx_msg BOOL OnEraseBkgnd(CDC* pDC);

	afx_msg void OnSetFocus(CWnd* pOldWnd);

	DECLARE_MESSAGE_MAP()

public:
	void OnIdleUpdate();

	afx_msg void OnTvnSelchangedTree(UINT id, NMHDR *pNMHDR, LRESULT *pResult);

	afx_msg void OnTvnDragchangedTree(UINT id, NMHDR *pNMHDR, LRESULT *pResult);

	void InvalidLayout(void)
	{
		m_bIsLayoutInvalid = TRUE;
	}
};

