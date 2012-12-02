#pragma once

class CRegion
{
public:
	CRect m_rc;

	CString m_name;

	COLORREF m_color;

	CImage m_image;

	CRegion(const CRect & rc, const CString & name, COLORREF color)
		: m_rc(rc)
		, m_name(name)
		, m_color(color)
	{
	}
};

class CRegionNode;

typedef boost::shared_ptr<CRegionNode> CRegionNodePtr;

typedef std::vector<CRegionNodePtr> CRegionNodePtrList;

class CRegionNode : public CRegion
{
public:
	CRegionNodePtrList m_childs;

	boost::weak_ptr<CRegionNode> m_Parent;

public:
	CRegionNode(const CRect & rc, const CString & name, COLORREF color)
		: CRegion(rc, name, color)
	{
	}

	static CRegionNodePtr InsertChild(CRegionNodePtr node, CRegionNodePtr child)
	{
		node->m_childs.push_back(child);

		child->m_Parent = node;

		return node;
	}

	static CRegionNodePtr GetPointedRegion(CRegionNodePtr root, const CPoint & ptLocal)
	{
		CRegionNodePtrList::const_iterator reg_iter = root->m_childs.begin();
		for(; reg_iter != root->m_childs.end(); reg_iter++)
		{
			CRegionNodePtr ret;
			if(ret = GetPointedRegion((*reg_iter), ptLocal - root->m_rc.TopLeft()))
				return ret;
		}

		if(root->m_rc.PtInRect(ptLocal))
			return root;

		return CRegionNodePtr();
	}
};

class CImgRegionDoc : public CDocument
{
public:
	CRegionNodePtr m_root;

	boost::weak_ptr<CRegionNode> m_SelectedNode;

public:
	DECLARE_DYNCREATE(CImgRegionDoc)

	CImgRegionDoc(void);

	virtual BOOL OnNewDocument(void);

	DECLARE_MESSAGE_MAP()

public:
	BOOL LocalToRoot(const CRegionNode * node, const CPoint & ptLocal, CPoint & ptResult);
};
