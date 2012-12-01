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
	friend class CImgRegionView;

protected:
	CRegionNodePtrList m_childs;

	boost::weak_ptr<CRegionNode> m_Parent;

public:
	CRegionNode(const CRect & rc, const CString & name, COLORREF color)
		: CRegion(rc, name, color)
	{
	}

	static void SetParent(CRegionNodePtr child, CRegionNodePtr parent);

	static CPoint LocalToRoot(CRegionNodePtr node, const CPoint & ptLocal);
};

class CImgRegionDoc : public CDocument
{
	DECLARE_DYNCREATE(CImgRegionDoc)

public:
	CImgRegionDoc(void);

	virtual BOOL OnNewDocument(void);

	CRegionNodePtr m_root;

	DECLARE_MESSAGE_MAP()
};
