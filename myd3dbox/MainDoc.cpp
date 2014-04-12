#include "StdAfx.h"
#include "MainDoc.h"
#include "MainApp.h"
#include "MainFrm.h"
#include "MainView.h"
#include "resource.h"
#include <rapidxml.hpp>
#include <rapidxml_print.hpp>
#include <fstream>
#include "a.hpp"

IMPLEMENT_DYNCREATE(CMainDoc, CDocument)

BEGIN_MESSAGE_MAP(CMainDoc, CDocument)
	ON_COMMAND(ID_EDIT_UNDO, &CMainDoc::OnEditUndo)
	ON_UPDATE_COMMAND_UI(ID_EDIT_UNDO, &CMainDoc::OnUpdateEditUndo)
	ON_COMMAND(ID_EDIT_REDO, &CMainDoc::OnEditRedo)
	ON_UPDATE_COMMAND_UI(ID_EDIT_REDO, &CMainDoc::OnUpdateEditRedo)
	ON_COMMAND(ID_CREATE_MESHFROMFILE, &CMainDoc::OnCreateMeshfromfile)
	ON_COMMAND(ID_CREATE_COLLISIONCAPSULE, &CMainDoc::OnCreateCollisioncapsule)
	ON_COMMAND(ID_CREATE_COLLISIONBOX, &CMainDoc::OnCreateCollisionbox)
	ON_COMMAND(ID_CREATE_JOINTREVOLUTE, &CMainDoc::OnCreateJointrevolute)
	ON_COMMAND(ID_CREATE_JOINTD6, &CMainDoc::OnCreateJointd6)
	ON_COMMAND(ID_FILE_EXPORT, &CMainDoc::OnExport)
END_MESSAGE_MAP()

CMainDoc::CMainDoc(void)
{
}

void CMainDoc::Clear(void)
{
	CHistoryMgr::ClearAllHistory();

	m_guid = 0;

	COutlinerView::getSingleton().m_TreeCtrl.DeleteAllItems();
}

void CMainDoc::Serialize(CArchive& ar)
{
	if(ar.IsStoring())
	{
		ar << m_guid;
	}
	else
	{
		ar >> m_guid;
	}

	COutlinerView::getSingleton().Serialize(ar);
}

BOOL CMainDoc::OnNewDocument()
{
	Clear();

	return CDocument::OnNewDocument();
}

BOOL CMainDoc::OnOpenDocument(LPCTSTR lpszPathName)
{
	Clear();

	if (!CDocument::OnOpenDocument(lpszPathName))
		return FALSE;

	return TRUE;
}

BOOL CMainDoc::OnSaveDocument(LPCTSTR lpszPathName)
{
	return CDocument::OnSaveDocument(lpszPathName);
}

void CMainDoc::OnCloseDocument()
{
	Clear();

	CDocument::OnCloseDocument();
}

void CMainDoc::OnEditUndo()
{
	CHistoryMgr::Undo();

	SetModifiedFlag();

	UpdateAllViews(NULL);
}

void CMainDoc::OnUpdateEditUndo(CCmdUI *pCmdUI)
{
	pCmdUI->Enable(m_nStep >= 0);
}

void CMainDoc::OnEditRedo()
{
	CHistoryMgr::Do();

	SetModifiedFlag();

	UpdateAllViews(NULL);
}

void CMainDoc::OnUpdateEditRedo(CCmdUI *pCmdUI)
{
	pCmdUI->Enable(m_nStep < (int)CHistoryMgr::size() - 1);
}

void CMainDoc::OnCreateMeshfromfile()
{
	CFileDialog dlg(TRUE);
	if(IDOK == dlg.DoModal())
	{
		try
		{
			AddTreeNodeMeshFromFile(dlg.GetPathName());

			SetModifiedFlag();

			UpdateAllViews(NULL);
		}
		catch (const my::Exception & e)
		{
			AfxMessageBox(str_printf(_T("Cannot open: %s\n%s"), dlg.GetFileName(), e.what().c_str()).c_str());
		}
	}
}

void CMainDoc::OnCreateCollisioncapsule()
{
	AddCollisionCapsule();

	SetModifiedFlag();

	UpdateAllViews(NULL);
}

void CMainDoc::OnCreateCollisionbox()
{
	AddCollisionBox();

	SetModifiedFlag();

	UpdateAllViews(NULL);
}

void CMainDoc::OnCreateJointrevolute()
{
	AddJointRevolute();

	SetModifiedFlag();

	UpdateAllViews(NULL);
}

void CMainDoc::OnCreateJointd6()
{
	AddJointD6();

	SetModifiedFlag();

	UpdateAllViews(NULL);
}

class RagdollExporter
{
public:
	rapidxml::xml_document<> m_doc;

	rapidxml::xml_node<> * m_root;

	rapidxml::xml_node<> * m_body_desc_list;

	int m_body_desc_list_size;

	boost::unordered_map<std::basic_string<TCHAR>, int> m_body_name_index_map;

	rapidxml::xml_node<> * m_joint_desc_list;

	int m_joint_desc_list_size;

	RagdollExporter(void)
		: m_body_desc_list_size(0)
		, m_joint_desc_list_size(0)
	{
		m_doc.append_node(m_doc.allocate_node(rapidxml::node_pi,m_doc.allocate_string("xml version='1.0' encoding='utf-8'")));
		m_root = m_doc.allocate_node(rapidxml::node_element, ROOT_NODE_NAME);
		m_doc.append_node(m_root);
		m_body_desc_list = m_doc.allocate_node(rapidxml::node_element, "m_body_desc_list");
		m_joint_desc_list = m_doc.allocate_node(rapidxml::node_element, "m_joint_desc_list");
		m_root->append_node(m_body_desc_list);
		m_root->append_node(m_joint_desc_list);
	}

	void PushStringElement(rapidxml::xml_node<> * node, const char * name, const std::string & value)
	{
		ASSERT(node);
		node->append_node(m_doc.allocate_node(rapidxml::node_element, name, m_doc.allocate_string(value.c_str())));
	}

	void PushFloatElement(rapidxml::xml_node<> * node, const char * name, float value)
	{
		ASSERT(node);
		node->append_node(m_doc.allocate_node(rapidxml::node_element, name, m_doc.allocate_string(str_printf("%f",value).c_str())));
	}

	void PushIntElement(rapidxml::xml_node<> * node, const char * name, int value)
	{
		ASSERT(node);
		node->append_node(m_doc.allocate_node(rapidxml::node_element, name, m_doc.allocate_string(str_printf("%d",value).c_str())));
	}

	void PushVector3Element(rapidxml::xml_node<> * node, const char * name, const my::Vector3 & value)
	{
		ASSERT(node);
		rapidxml::xml_node<> * vect = m_doc.allocate_node(rapidxml::node_element, name);
		vect->append_attribute(m_doc.allocate_attribute("class_name", "vector3"));
		PushFloatElement(vect, "x", value.x);
		PushFloatElement(vect, "y", value.y);
		PushFloatElement(vect, "z", value.z);
		node->append_node(vect);
	}

	void PushQuaterionElement(rapidxml::xml_node<> * node, const char * name, const my::Quaternion & value)
	{
		ASSERT(node);
		rapidxml::xml_node<> * quat = m_doc.allocate_node(rapidxml::node_element, name);
		quat->append_attribute(m_doc.allocate_attribute("class_name", "quat"));
		PushFloatElement(quat, "x", value.x);
		PushFloatElement(quat, "y", value.y);
		PushFloatElement(quat, "z", value.z);
		PushFloatElement(quat, "w", value.w);
		node->append_node(quat);
	}

	void PushBodyCapsule(TreeNodeCollisionCapsulePtr node, const std::basic_string<TCHAR> & text)
	{
		ASSERT(node);
		rapidxml::xml_node<> * array_item = m_doc.allocate_node(rapidxml::node_element, "array_item");
		array_item->append_attribute(m_doc.allocate_attribute("class_name", BODY_CAPSULE_CLASS_NAME));
		PushStringElement(array_item, "bone_name", tstou8((LPCTSTR)node->m_BindBone));
		PushVector3Element(array_item, "pos", node->m_Position);
		PushQuaterionElement(array_item, "rot", node->m_Rotation);
		PushFloatElement(array_item, "radius", node->m_Radius);
		PushFloatElement(array_item, "half_height", node->m_Height * 0.5f);
		PushFloatElement(array_item, "density", 1.0f);
		m_body_desc_list->append_node(array_item);
		m_body_name_index_map[text] = m_body_desc_list_size++;
	}

	void PushBodyBox(TreeNodeCollisionBoxPtr node, const std::basic_string<TCHAR> & text)
	{
		ASSERT(node);
		rapidxml::xml_node<> * array_item = m_doc.allocate_node(rapidxml::node_element, "array_item");
		array_item->append_attribute(m_doc.allocate_attribute("class_name", BODY_BOX_CLASS_NAME));
		PushStringElement(array_item, "bone_name", tstou8((LPCTSTR)node->m_BindBone));
		PushVector3Element(array_item, "pos", node->m_Position);
		PushQuaterionElement(array_item, "rot", node->m_Rotation);
		PushVector3Element(array_item, "half_extent", (my::Vector3&)node->m_Box.GetExtents());
		PushFloatElement(array_item, "density", 1.0f);
		m_body_desc_list->append_node(array_item);
		m_body_name_index_map[text] =m_body_desc_list_size++;
	}

	void PushJointRevolute(TreeNodeJointRevolutePtr node, const std::basic_string<TCHAR> & text)
	{
		ASSERT(node);
		rapidxml::xml_node<> * array_item = m_doc.allocate_node(rapidxml::node_element, "array_item");
		array_item->append_attribute(m_doc.allocate_attribute("class_name", JOINT_REVOLUTE_CLASS_NAME));
		boost::unordered_map<std::basic_string<TCHAR>, int>::const_iterator iter;
		int body0 = ((iter = m_body_name_index_map.find((LPCTSTR)node->m_Body0)) != m_body_name_index_map.end()) ? iter->second
			: (theApp.OnResourceFailed(str_printf(_T("Joint \"%s\" error, cannot find body0 \"%s\""), text.c_str(), (LPCTSTR)node->m_Body0)), -1);
		int body1 = ((iter = m_body_name_index_map.find((LPCTSTR)node->m_Body1)) != m_body_name_index_map.end()) ? iter->second
			: (theApp.OnResourceFailed(str_printf(_T("Joint \"%s\" error, cannot find body1 \"%s\""), text.c_str(), (LPCTSTR)node->m_Body1)), -1);
		if (body0 >= 0 && body0 == body1)
			theApp.OnResourceFailed(str_printf(_T("Joint \"%s\" error, body0 and body1 must not be the same"), text.c_str()));
		PushIntElement(array_item, "body0", body0);
		PushIntElement(array_item, "body1", body1);
		PushVector3Element(array_item, "pos", node->m_Position);
		PushQuaterionElement(array_item, "rot", node->m_Rotation);
		PushFloatElement(array_item, "lower_limit", node->m_LowerLimit);
		PushFloatElement(array_item, "upper_limit", node->m_UpperLimit);
		PushFloatElement(array_item, "limit_constant_dist", 0.001f);
		m_joint_desc_list->append_node(array_item);
		m_joint_desc_list_size++;
	}

	void PushJointD6(TreeNodeJointD6Ptr node, const std::basic_string<TCHAR> & text)
	{
		ASSERT(node);
		rapidxml::xml_node<> * array_item = m_doc.allocate_node(rapidxml::node_element, "array_item");
		array_item->append_attribute(m_doc.allocate_attribute("class_name", JOINT_D6_CLASS_NAME));
		boost::unordered_map<std::basic_string<TCHAR>, int>::const_iterator iter;
		int body0 = ((iter = m_body_name_index_map.find((LPCTSTR)node->m_Body0)) != m_body_name_index_map.end()) ? iter->second
			: (theApp.OnResourceFailed(str_printf(_T("Joint \"%s\" error, cannot find body0 \"%s\""), text.c_str(), (LPCTSTR)node->m_Body0)), -1);
		int body1 = ((iter = m_body_name_index_map.find((LPCTSTR)node->m_Body1)) != m_body_name_index_map.end()) ? iter->second
			: (theApp.OnResourceFailed(str_printf(_T("Joint \"%s\" error, cannot find body1 \"%s\""), text.c_str(), (LPCTSTR)node->m_Body1)), -1);
		if (body0 >= 0 && body0 == body1)
			theApp.OnResourceFailed(str_printf(_T("Joint \"%s\" error, body0 and body1 must not be the same"), text.c_str()));
		PushIntElement(array_item, "body0", body0);
		PushIntElement(array_item, "body1", body1);
		PushVector3Element(array_item, "pos", node->m_Position);
		PushQuaterionElement(array_item, "rot", node->m_Rotation);
		PushFloatElement(array_item, "twist_min", node->m_TwistMin);
		PushFloatElement(array_item, "twist_max", node->m_TwistMax);
		PushFloatElement(array_item, "y_limit_angle", node->m_YLimitAngle);
		PushFloatElement(array_item, "z_limit_angle", node->m_ZLimitAngle);
		PushFloatElement(array_item, "limit_constant_dist", 0.001f);
		m_joint_desc_list->append_node(array_item);
		m_joint_desc_list_size++;
	}

	void DoExport(LPCTSTR szPath)
	{
		std::string body_desc_list_size = str_printf("%d",m_body_desc_list_size);
		m_body_desc_list->append_attribute(m_doc.allocate_attribute("array_size", body_desc_list_size.c_str()));
		std::string joint_desc_list_size = str_printf("%d",m_joint_desc_list_size);
		m_joint_desc_list->append_attribute(m_doc.allocate_attribute("array_size", joint_desc_list_size.c_str()));

		std::ofstream out(szPath);
		out << m_doc;
	}
};

void CMainDoc::OnExport()
{
	CFileDialog dlg(FALSE);
	if(IDOK == dlg.DoModal())
	{
		RagdollExporter exporter;

		COutlinerView * pOutline = COutlinerView::getSingletonPtr();
		HTREEITEM hItem = pOutline->m_TreeCtrl.GetRootItem();
		for(; hItem; hItem = pOutline->m_TreeCtrl.GetNextSiblingItem(hItem))
		{
			TreeNodeBasePtr node = pOutline->GetItemNode(hItem);
			if(TreeNodeCollisionCapsulePtr sub_node = boost::dynamic_pointer_cast<TreeNodeCollisionCapsule>(node))
			{
				exporter.PushBodyCapsule(sub_node, (LPCTSTR)pOutline->m_TreeCtrl.GetItemText(hItem));
			}
			else if(TreeNodeCollisionBoxPtr sub_node = boost::dynamic_pointer_cast<TreeNodeCollisionBox>(node))
			{
				exporter.PushBodyBox(sub_node, (LPCTSTR)pOutline->m_TreeCtrl.GetItemText(hItem));
			}
		}

		for(hItem = pOutline->m_TreeCtrl.GetRootItem(); hItem; hItem = pOutline->m_TreeCtrl.GetNextSiblingItem(hItem))
		{
			TreeNodeBasePtr node = pOutline->GetItemNode(hItem);
			if(TreeNodeJointRevolutePtr sub_node = boost::dynamic_pointer_cast<TreeNodeJointRevolute>(node))
			{
				exporter.PushJointRevolute(sub_node, (LPCTSTR)pOutline->m_TreeCtrl.GetItemText(hItem));
			}
			else if(TreeNodeJointD6Ptr sub_node = boost::dynamic_pointer_cast<TreeNodeJointD6>(node))
			{
				exporter.PushJointD6(sub_node, (LPCTSTR)pOutline->m_TreeCtrl.GetItemText(hItem));
			}
		}

		exporter.DoExport(dlg.GetPathName());
	}
}
