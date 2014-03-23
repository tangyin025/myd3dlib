#pragma once

#include "TreeNode.h"

class CStepBase
{
public:
	CStepBase(void)
	{
	}

	virtual ~CStepBase(void)
	{
	}

	virtual void Do(void) = 0;
};

typedef boost::shared_ptr<CStepBase> CStepBasePtr;

class CAddTreeNodeStep : public CStepBase
{
public:
	UINT m_itemID;

	std::basic_string<TCHAR> m_strItem;

	TreeNodeBasePtr m_node;

	UINT m_parentID;

	UINT m_beforeID;

	CAddTreeNodeStep(
		UINT itemID,
		LPCTSTR lpszItem,
		TreeNodeBasePtr node,
		UINT parentID = 0,
		UINT beforeID = 0)
		: m_itemID(itemID)
		, m_strItem(lpszItem)
		, m_node(node)
		, m_parentID(parentID)
		, m_beforeID(beforeID)
	{
	}

	virtual void Do(void);
};

class CDeleteTreeNodeStep : public CStepBase
{
public:
	UINT m_itemID;

	std::basic_string<TCHAR> m_strItem;

	TreeNodeBasePtr m_node;

	UINT m_parentID;

	UINT m_beforeID;

	CDeleteTreeNodeStep(UINT itemID)
		: m_itemID(itemID)
		, m_parentID(0)
		, m_beforeID(0)
	{
	}

	virtual void Do(void);
};

class CHistory
	: public std::vector<std::pair<CStepBasePtr, CStepBasePtr> >
{
public:
	CHistory(void)
	{
	}

	virtual ~CHistory(void)
	{
	}

	void Do(void);

	void Undo(void);
};

typedef boost::shared_ptr<CHistory> CHistoryPtr;

class CHistoryMgr
	: protected std::deque<CHistoryPtr>
{
public:
	UINT m_guid;

	int m_nStep;

public:
	CHistoryMgr(void)
		: m_guid(0)
		, m_nStep(-1)
	{
	}

	virtual ~CHistoryMgr(void)
	{
		ASSERT(empty());
	}

	void Do(void);

	void Undo(void);

	void ClearAllHistory(void);

	void AddHistory(CHistoryPtr hist);

	void AddTreeNode(LPCTSTR lpszItem, TreeNodeBasePtr node);

	void DeleteTreeNode(HTREEITEM hItem);

	void AddTreeNodeMeshFromFile(LPCTSTR lpszMesh);

	void AddCollisionCapsule(void);
};
