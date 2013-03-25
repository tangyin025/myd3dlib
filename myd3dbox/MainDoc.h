#pragma once

#include "OutlinerView.h"

class TreeStaticMeshNode : TreeNodeBase
{
public:
	TreeStaticMeshNode(my::OgreMeshPtr mesh)
		: m_mesh(mesh)
	{
	}

	TreeStaticMeshNode(void)
	{
	}

	my::OgreMeshPtr m_mesh;

	virtual void Draw(IDirect3DDevice9 * pd3dDevice, float fElapsedTime);
};

class CMainDoc
	: public CDocument
	, public my::SingleInstance<CMainDoc>
{
public:
	CMainDoc(void);

	DECLARE_DYNCREATE(CMainDoc)

	boost::shared_ptr<btCollisionShape> m_groundShape;

	boost::shared_ptr<btMotionState> m_groundMotionState;

	boost::shared_ptr<btRigidBody> m_groundBody;

	virtual BOOL OnNewDocument();

	virtual BOOL OnOpenDocument(LPCTSTR lpszPathName);

	virtual BOOL OnSaveDocument(LPCTSTR lpszPathName);

	virtual void OnCloseDocument();

	DECLARE_MESSAGE_MAP()

	afx_msg void OnCreateStaticmesh();
};
