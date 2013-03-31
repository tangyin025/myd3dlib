#pragma once

class TreeNodeBase
{
public:
	TreeNodeBase(void)
	{
	}

	virtual ~TreeNodeBase(void)
	{
	}

	virtual btRigidBody * GetRigidBody(void) { return NULL; }

	virtual void Draw(IDirect3DDevice9 * pd3dDevice, float fElapsedTime) = 0;
};

typedef boost::shared_ptr<TreeNodeBase> TreeNodeBasePtr;

class COutlinerTreeCtrl : public CTreeCtrl
{
public:
	COutlinerTreeCtrl(void)
		: m_bDrag(FALSE)
		, m_hDragItem(NULL)
		, m_DragDropType(DropTypeNone)
	{
	}

	DECLARE_MESSAGE_MAP()

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

	virtual BOOL OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult);

	afx_msg void OnTvnBegindrag(NMHDR *pNMHDR, LRESULT *pResult);

	afx_msg void OnMouseMove(UINT nFlags, CPoint point);

	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);

	afx_msg void OnNMCustomdraw(NMHDR *pNMHDR, LRESULT *pResult);

	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
};

class COutlinerView
	: public CDockablePane
	, public my::SingleInstance<COutlinerView>
{
public:
	COutlinerView(void)
	{
	}

	virtual ~COutlinerView(void)
	{
		ASSERT(m_ItemMap.empty());
	}

	DECLARE_MESSAGE_MAP()
public:
	COutlinerTreeCtrl m_TreeCtrl;

	CMFCToolBar m_wndToolBar;

	typedef std::tr1::unordered_map<std::basic_string<TCHAR>, HTREEITEM, boost::hash<std::basic_string<TCHAR> > > TreeItemMap;

	TreeItemMap m_ItemMap;

	boost::shared_ptr<btDefaultCollisionConfiguration> m_collisionConfiguration;

	boost::shared_ptr<btCollisionDispatcher> m_dispatcher;

	boost::shared_ptr<btBroadphaseInterface> m_overlappingPairCache;

	boost::shared_ptr<btConstraintSolver> m_constraintSolver;

	boost::shared_ptr<btDiscreteDynamicsWorld> m_dynamicsWorld;

	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);

	virtual void AdjustLayout(void);

	virtual BOOL PreTranslateMessage(MSG* pMsg);

	afx_msg void OnSize(UINT nType, int cx, int cy);

	afx_msg void OnTvnSelchangedTree(NMHDR *pNMHDR, LRESULT *pResult);

	afx_msg void OnTvnDragchangedTree(NMHDR *pNMHDR, LRESULT *pResult);

	afx_msg void OnTvnDeleteitem(NMHDR *pNMHDR, LRESULT *pResult);

	afx_msg void OnTvnBeginlabeledit(NMHDR *pNMHDR, LRESULT *pResult);

	afx_msg void OnTvnEndlabeledit(NMHDR *pNMHDR, LRESULT *pResult);

	afx_msg void OnTvnUserDeleting(NMHDR *pNMHDR, LRESULT *pResult);

	void InsertItem(const std::basic_string<TCHAR> & strItem, TreeNodeBasePtr node, HTREEITEM hParent = TVI_ROOT, HTREEITEM hInsertAfter = TVI_LAST);

	TreeNodeBasePtr GetItemNode(HTREEITEM hItem);

	void DrawItemNode(IDirect3DDevice9 * pd3dDevice, float fElapsedTime, HTREEITEM hItem);
};
