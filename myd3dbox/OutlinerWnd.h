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
	typedef std::map<Component *, HTREEITEM> Cmp2HTreeMap;
	Cmp2HTreeMap m_Cmp2HTree;

	struct TreeItemData
	{
		DWORD type;

		void * data;

		TreeItemData(DWORD _type, void * _data)
			: type(_type)
			, data(_data)
		{
		}

		template <typename T>
		T * reinterpret_cast_data(void)
		{
			return reinterpret_cast<T*>(data);
		}
	};

	enum TreeItemType
	{
		TreeItemTypeActor,
		TreeItemTypeComponent,
	};

	BOOL CanTreeItemMove(HTREEITEM hMoveItem, HTREEITEM hParent, HTREEITEM hInsertAfter);
	HTREEITEM InsertTreeItem(LPCTSTR strItem, DWORD type, void * data, int nImage, int nSelectedImage, HTREEITEM hParent = TVI_ROOT, HTREEITEM hInsertAfter = TVI_LAST);
	void DeleteTreeItem(HTREEITEM hItem);
	HTREEITEM MoveTreeItem(HTREEITEM hMoveItem, HTREEITEM hParent, HTREEITEM hInsertAfter);
	//void InsertActor(Actor * actor, HTREEITEM hParent = TVI_ROOT, HTREEITEM hInsertAfter = TVI_LAST);
	//void InsertComponent(Component * cmp, HTREEITEM hParent = TVI_ROOT, HTREEITEM hInsertAfter = TVI_LAST);

// Overrides
public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);

protected:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	afx_msg void OnPaint();
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	afx_msg LRESULT OnChangeActiveTab(WPARAM, LPARAM);

	DECLARE_MESSAGE_MAP()
};
