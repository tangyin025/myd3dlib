#pragma once

#include "../demo2_3/Component/ActorComponent.h"
#include "MltiTree.h"

class Actor;

class CClassToolBar : public CMFCToolBar
{
	virtual void OnUpdateCmdUI(CFrameWnd* /*pTarget*/, BOOL bDisableIfNoHndler)
	{
		CMFCToolBar::OnUpdateCmdUI((CFrameWnd*) GetOwner(), bDisableIfNoHndler);
	}

	virtual BOOL AllowShowOnList() const { return FALSE; }
};

class CTC : public CMultiTree
{
#ifdef _DEBUG
	//skip assert because of dual-mapped CWnd
	//virtual void AssertValid() const {};
	//if a non-CTC derived CTreeCtrl class is used
	// then make sure the AssertValid doesn't call
	// the base class
#endif
	//need access to m_pfnSuper
	friend class COutlinerWnd;
public:
	DECLARE_MESSAGE_MAP()
	afx_msg void OnNMCustomdraw(NMHDR *pNMHDR, LRESULT *pResult);
};

class COutlinerWnd : public CDockablePane
{
public:
	COutlinerWnd();
	virtual ~COutlinerWnd();

	void AdjustLayout();
	void OnChangeVisualStyle();

public:
	CTC m_wndClassView;
	CImageList m_ClassViewImages;
	CMenu m_ContextMenu;
	CMenu m_ContextMenuAdd;

// Overrides
public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);

	typedef std::map<DWORD_PTR, HTREEITEM> Data2HTreeMap;

	Data2HTreeMap m_Data2HTree;

	//BOOL CanTreeItemMove(HTREEITEM hMoveItem, HTREEITEM hParent, HTREEITEM hInsertAfter);
	HTREEITEM InsertTreeItem(LPCTSTR strItem, DWORD_PTR pData, int nImage, int nSelectedImage, HTREEITEM hParent = TVI_ROOT, HTREEITEM hInsertAfter = TVI_LAST);
	void DeleteTreeItem(HTREEITEM hItem);
	void DeleteAllTreeItems(void);
	//HTREEITEM MoveTreeItem(HTREEITEM hMoveItem, HTREEITEM hParent, HTREEITEM hInsertAfter);
	void InsertComponent(Component * cmp, HTREEITEM hParent = TVI_ROOT, HTREEITEM hInsertAfter = TVI_LAST);
	template <typename T>
	T GetTreeItemData(HTREEITEM hItem)
	{
		return reinterpret_cast<T>(m_wndClassView.GetItemData(hItem));
	}
	HTREEITEM GetTreeItemByData(DWORD_PTR pData);

protected:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	afx_msg void OnPaint();
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	afx_msg LRESULT OnChangeActiveTab(WPARAM, LPARAM);
	afx_msg void OnTvnSelchangedTree(NMHDR *pNMHDR, LRESULT *pResult);

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnMenuAddMeshComponent();
};
