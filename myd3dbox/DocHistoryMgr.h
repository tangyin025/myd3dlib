#pragma once

#include "TreeNode.h"

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

class CAddTreeNodeStep : public CDocStepBase
{
public:
	std::basic_string<TCHAR> m_strItem;

	TreeNodeBasePtr m_node;

	std::basic_string<TCHAR> m_strParent;

	std::basic_string<TCHAR> m_strBefore;

	CAddTreeNodeStep(
		LPCTSTR lpszItem,
		TreeNodeBasePtr node,
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

class CDeleteTreeNodeStep : public CDocStepBase
{
public:
	std::basic_string<TCHAR> m_strItem;

	TreeNodeBasePtr m_node;

	CDeleteTreeNodeStep(LPCTSTR lpszItem)
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
		ASSERT(empty());
	}

	int m_nStep;

	void Do(void);

	void Undo(void);

	void ClearAllHistory(void);

	void AddHistory(CDocHistoryPtr hist);

	void AddTreeNode(LPCTSTR lpszItem, TreeNodeBasePtr node);

	void DeleteTreeNode(HTREEITEM hItem);

	void AddStaticMeshTreeNode(LPCTSTR lpszMesh);
};
