#pragma once

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
};
