#pragma once

typedef boost::shared_ptr<Gdiplus::Image> ImagePtr;

typedef boost::shared_ptr<Gdiplus::Font> FontPtr2;

class Vector4i
{
public:
	int x, y, z, w;

	Vector4i(int _x, int _y, int _z, int _w)
		: x(_x), y(_y), z(_z), w(_w)
	{
	}

	Vector4i(void)
	{
	}
};

class CImgRegion
{
public:
	CRect m_rc;

	CString m_name;

	Gdiplus::Color m_color;

	ImagePtr m_image;

	Vector4i m_border;

	FontPtr2 m_font;

	CImgRegion(const CRect & rc, const CString & name, const Gdiplus::Color & color)
		: m_rc(rc)
		, m_name(name)
		, m_color(color)
		, m_border(0,0,0,0)
	{
	}
};

class CImgRegionNode;

typedef boost::shared_ptr<CImgRegionNode> CImgRegionNodePtr;

typedef std::vector<CImgRegionNodePtr> CImgRegionNodePtrList;

class CImgRegionNode : public CImgRegion
{
public:
	CImgRegionNodePtrList m_childs;

	boost::weak_ptr<CImgRegionNode> m_Parent;

public:
	CImgRegionNode(const CRect & rc, const CString & name, const Gdiplus::Color & color)
		: CImgRegion(rc, name, color)
	{
	}

	static CImgRegionNodePtr InsertChild(CImgRegionNodePtr node, CImgRegionNodePtr child)
	{
		node->m_childs.push_back(child);

		child->m_Parent = node;

		return node;
	}

	static CImgRegionNodePtr GetPointedRegion(CImgRegionNodePtr root, const CPoint & ptLocal)
	{
		CImgRegionNodePtrList::const_iterator reg_iter = root->m_childs.begin();
		for(; reg_iter != root->m_childs.end(); reg_iter++)
		{
			CImgRegionNodePtr ret;
			if(ret = GetPointedRegion((*reg_iter), ptLocal - root->m_rc.TopLeft()))
				return ret;
		}

		if(root->m_rc.PtInRect(ptLocal))
			return root;

		return CImgRegionNodePtr();
	}
};

class CImgRegionDoc : public CDocument
{
public:
	CImgRegionNodePtr m_root;

	boost::weak_ptr<CImgRegionNode> m_SelectedNode;

public:
	DECLARE_DYNCREATE(CImgRegionDoc)

	CImgRegionDoc(void);

	virtual BOOL OnNewDocument(void);

	DECLARE_MESSAGE_MAP()

public:
	BOOL LocalToRoot(const CImgRegionNode * node, const CPoint & ptLocal, CPoint & ptResult);
};
