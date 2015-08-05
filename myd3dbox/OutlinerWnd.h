#pragma once

#include "DragableTreeCtrl.h"
#include "Component/ActorComponent.h"

class Actor;

class CClassToolBar : public CMFCToolBar
{
	virtual void OnUpdateCmdUI(CFrameWnd* /*pTarget*/, BOOL bDisableIfNoHndler)
	{
		CMFCToolBar::OnUpdateCmdUI((CFrameWnd*) GetOwner(), bDisableIfNoHndler);
	}

	virtual BOOL AllowShowOnList() const { return FALSE; }
};

class COutlinerWnd : public CDockablePane
{
public:
	COutlinerWnd();
	virtual ~COutlinerWnd();

	void AdjustLayout();
	void OnChangeVisualStyle();

protected:
	CDragableTreeCtrl m_wndClassView;
	CImageList m_ClassViewImages;

// Overrides
public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);

	struct TreeItemData
	{
		DWORD Type;

		void * Data;

		TreeItemData(DWORD _Type, void * _Data)
			: Type(_Type)
			, Data(_Data)
		{
		}

		template <typename T>
		T * reinterpret_cast_data(void)
		{
			return reinterpret_cast<T*>(Data);
		}
	};

	enum TreeItemType
	{
		TreeItemTypeActor,
		TreeItemTypeComponent,
	};

	typedef std::map<void *, HTREEITEM> Data2HTreeMap;

	Data2HTreeMap m_Data2HTree;

	BOOL CanTreeItemMove(HTREEITEM hMoveItem, HTREEITEM hParent, HTREEITEM hInsertAfter);
	HTREEITEM InsertTreeItem(LPCTSTR strItem, DWORD type, void * data, int nImage, int nSelectedImage, HTREEITEM hParent = TVI_ROOT, HTREEITEM hInsertAfter = TVI_LAST);
	void DeleteTreeItem(HTREEITEM hItem);
	HTREEITEM MoveTreeItem(HTREEITEM hMoveItem, HTREEITEM hParent, HTREEITEM hInsertAfter);
	void InsertActor(Actor * actor, HTREEITEM hParent = TVI_ROOT, HTREEITEM hInsertAfter = TVI_LAST);
	void InsertComponent(Component * cmp, HTREEITEM hParent = TVI_ROOT, HTREEITEM hInsertAfter = TVI_LAST);

protected:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	afx_msg void OnPaint();
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	afx_msg LRESULT OnChangeActiveTab(WPARAM, LPARAM);

	DECLARE_MESSAGE_MAP()
};
