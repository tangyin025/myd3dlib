
#pragma once

#include "EventDefine.h"
#include "Component/Component.h"

class CPropertiesToolBar : public CMFCToolBar
{
public:
	virtual void OnUpdateCmdUI(CFrameWnd* /*pTarget*/, BOOL bDisableIfNoHndler)
	{
		CMFCToolBar::OnUpdateCmdUI((CFrameWnd*) GetOwner(), bDisableIfNoHndler);
	}

	virtual BOOL AllowShowOnList() const { return FALSE; }
};

class CPropertiesWnd : public CDockablePane
{
// Construction
public:
	CPropertiesWnd();

	void AdjustLayout();

// Attributes
public:
	void SetVSDotNetLook(BOOL bSet)
	{
		m_wndPropList.SetVSDotNetLook(bSet);
		m_wndPropList.SetGroupNameFullWidth(bSet);
	}

protected:
	CFont m_fntPropList;
	CComboBox m_wndObjectCombo;
	//CPropertiesToolBar m_wndToolBar;
	CMFCPropertyGridCtrl m_wndPropList;
	enum Property
	{
		PropertyUnknown = 0,
		PropertyComponent,
		PropertyComponentMinX,
		PropertyComponentMinY,
		PropertyComponentMinZ,
		PropertyComponentMaxX,
		PropertyComponentMaxY,
		PropertyComponentMaxZ,
		PropertyComponentPosX,
		PropertyComponentPosY,
		PropertyComponentPosZ,
		PropertyComponentRotX,
		PropertyComponentRotY,
		PropertyComponentRotZ,
		PropertyComponentScaleX,
		PropertyComponentScaleY,
		PropertyComponentScaleZ,
		PropertyMesh,
		PropertyMeshPath,
		PropertyEmitter,
		PropertyEmitterParticleLifeTime,
		PropertySphericalEmitter,
		PropertySphericalEmitterSpawnInterval,
		PropertySphericalEmitterHalfSpawnAreaX,
		PropertySphericalEmitterHalfSpawnAreaY,
		PropertySphericalEmitterHalfSpawnAreaZ,
		PropertySphericalEmitterSpawnSpeed,
		PropertySphericalEmitterSpawnInclination,
		PropertySphericalEmitterSpawnAzimuth,
		PropertySphericalEmitterSpawnColorA,
		PropertySphericalEmitterSpawnColorR,
		PropertySphericalEmitterSpawnColorG,
		PropertySphericalEmitterSpawnColorB,
		PropertySphericalEmitterSpawnSizeX,
		PropertySphericalEmitterSpawnSizeY,
		PropertySphericalEmitterSpawnAngle,
		PropertySphericalEmitterSpawnLoopTime,
		PropertySplineNodeCount,
		PropertySplineNodeX,
		PropertySplineNodeY,
		PropertySplineNodeK0,
		PropertySplineNodeK,
		PropertyTerrain,
		PropertyTerrainTexStartX,
		PropertyTerrainTexStartY,
		PropertyTerrainTexEndX,
		PropertyTerrainTexEndY,
		PropertyTerrainXDivision,
		PropertyTerrainZDivision,
		PropertyMaterial0,
		PropertyMaterial1,
		PropertyMaterial2,
		PropertyMaterial3,
		PropertyMaterial4,
		PropertyMaterial5,
		PropertyMaterial6,
		PropertyMaterial7,
		PropertyMaterial8,
		PropertyMaterial9,
		PropertyMaterialEnd,
		PropertyMaterialShader,
		PropertyMaterialPassMask,
		PropertyMaterialMeshTexture,
		PropertyMaterialNormalTexture,
		PropertyMaterialSpecularTexture,
		PropertyCount
	};
	CMFCPropertyGridProperty * m_pProp[PropertyCount];

	void OnSelectionChanged(EventArg * arg);
	void OnCmpAttriChanged(EventArg * arg);
	void UpdateProperties(Component * cmp);
	void UpdatePropertiesMesh(MeshComponent * cmp);
	void UpdatePropertiesMaterial(Material * mat, Property PropertyId);
	void CreatePropertiesSpline(CMFCPropertyGridProperty * pParentProp, LPCTSTR lpszName, Property PropertyId);
	void CreatePropertiesSplineNode(CMFCPropertyGridProperty * pParentProp, DWORD NodeId);
	void CreatePropertiesMaterial(CMFCPropertyGridCtrl * pParentCtrl, LPCTSTR lpszName, Property PropertyId);

// Implementation
public:
	virtual ~CPropertiesWnd();

protected:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnDestroy();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	//afx_msg void OnExpandAllProperties();
	//afx_msg void OnUpdateExpandAllProperties(CCmdUI* pCmdUI);
	//afx_msg void OnSortProperties();
	//afx_msg void OnUpdateSortProperties(CCmdUI* pCmdUI);
	//afx_msg void OnProperties1();
	//afx_msg void OnUpdateProperties1(CCmdUI* pCmdUI);
	//afx_msg void OnProperties2();
	//afx_msg void OnUpdateProperties2(CCmdUI* pCmdUI);
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	afx_msg void OnSettingChange(UINT uFlags, LPCTSTR lpszSection);

	DECLARE_MESSAGE_MAP()

	void InitPropList();
	void SetPropListFont();
	afx_msg LRESULT OnPropertyChanged(WPARAM wParam, LPARAM lParam);
public:
};

