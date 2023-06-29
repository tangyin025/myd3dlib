#pragma once

#include "Component.h"
#include "DebugDraw.h"

// CSnapshotDlg dialog

class CSnapshotDlg
	: public CDialogEx
	, public my::DrawHelper
	, public duDebugDraw
{
	DECLARE_DYNAMIC(CSnapshotDlg)

public:
	CSnapshotDlg(CWnd* pParent = nullptr);   // standard constructor
	virtual ~CSnapshotDlg();

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOG7 };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual void depthMask(bool state);
	virtual void texture(bool state);
	virtual void begin(duDebugDrawPrimitives prim, float size = 1.0f);
	virtual void vertex(const float* pos, unsigned int color);
	virtual void vertex(const float x, const float y, const float z, unsigned int color);
	virtual void vertex(const float* pos, unsigned int color, const float* uv);
	virtual void vertex(const float x, const float y, const float z, unsigned int color, const float u, const float v);
	virtual void end();
	virtual unsigned int areaToCol(unsigned int area);

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	CString m_TexPath;
	int m_TexWidth;
	int m_TexHeight;
	my::Rectangle m_SnapArea;
	my::Vector3 m_SnapEye;
	my::Vector3 m_SnapEular;
	afx_msg void OnClickedButton1();
	BOOL m_ComponentTypes[Component::ComponentTypeNavigation - Component::ComponentTypeMesh + 1];
	DWORD m_duDebugDrawPrimitives;
	int m_RTType;
};
