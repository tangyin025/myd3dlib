#pragma once

#include "OutlinerView.h"

class StaticMeshTreeNode : public TreeNodeBase
{
public:
	StaticMeshTreeNode(my::OgreMeshPtr mesh)
		: m_mesh(mesh)
	{
	}

	StaticMeshTreeNode(void)
	{
	}

	my::OgreMeshPtr m_mesh;

	virtual void Draw(IDirect3DDevice9 * pd3dDevice, float fElapsedTime);
};

typedef boost::shared_ptr<StaticMeshTreeNode> StaticMeshTreeNodePtr;

class CDocStepBase
{
public:
	CDocStepBase(void)
	{
	}

	virtual ~CDocStepBase(void)
	{
	}

	virtual void Do(void) = 0;
};

typedef boost::shared_ptr<CDocStepBase> CDocStepBasePtr;

class CAddStaticMeshTreeNodeStep : public CDocStepBase
{
public:
	std::basic_string<TCHAR> m_strItem;

	StaticMeshTreeNodePtr m_node;

	std::basic_string<TCHAR> m_strParent;

	std::basic_string<TCHAR> m_strBefore;

	CAddStaticMeshTreeNodeStep(
		LPCTSTR lpszItem,
		StaticMeshTreeNodePtr node,
		LPCTSTR lpszParent = _T(""),
		LPCTSTR lpszBefore = _T(""))
		: m_strItem(lpszItem)
		, m_node(node)
		, m_strParent(lpszParent)
		, m_strBefore(lpszBefore)
	{
	}

	virtual void Do(void);
};

class CDeleteStaticMeshTreeNodeStep : public CDocStepBase
{
public:
	std::basic_string<TCHAR> m_strItem;

	StaticMeshTreeNodePtr m_node;

	std::basic_string<TCHAR> m_strParent;

	std::basic_string<TCHAR> m_strBefore;

	CDeleteStaticMeshTreeNodeStep(LPCTSTR lpszItem)
		: m_strItem(lpszItem)
	{
	}

	virtual void Do(void);
};

class CDocHistory
	: public std::vector<std::pair<CDocStepBasePtr, CDocStepBasePtr> >
{
public:
	CDocHistory(void)
	{
	}

	virtual ~CDocHistory(void)
	{
	}

	void Do(void);

	void Undo(void);
};

typedef boost::shared_ptr<CDocHistory> CDocHistoryPtr;

class CDocHistoryMgr
	: protected std::deque<CDocHistoryPtr>
{
public:
	CDocHistoryMgr(void)
		: m_nStep(-1)
	{
	}

	virtual ~CDocHistoryMgr(void)
	{
	}

	int m_nStep;

	void AddTreeStaticMeshNode(LPCTSTR lpszItem, my::OgreMeshPtr mesh);
};
