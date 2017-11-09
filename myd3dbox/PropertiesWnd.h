
#pragma once

#include "EventDefine.h"
#include "Component/Character.h"
#include "Component/Terrain.h"

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
		PropertyActor,
		PropertyActorLevelId,
		PropertyActorLevelIdX,
		PropertyActorLevelIdY,
		PropertyActorAABB,
		PropertyActorMinX,
		PropertyActorMinY,
		PropertyActorMinZ,
		PropertyActorMaxX,
		PropertyActorMaxY,
		PropertyActorMaxZ,
		PropertyActorPos,
		PropertyActorPosX,
		PropertyActorPosY,
		PropertyActorPosZ,
		PropertyActorRot,
		PropertyActorRotX,
		PropertyActorRotY,
		PropertyActorRotZ,
		PropertyActorScale,
		PropertyActorScaleX,
		PropertyActorScaleY,
		PropertyActorScaleZ,
		PropertyActorRigidActor,
		PropertyComponent,
		PropertyComponentShape,
		//PropertyComponentShapePos,
		//PropertyComponentShapePosX,
		//PropertyComponentShapePosY,
		//PropertyComponentShapePosZ,
		//PropertyComponentShapeRot,
		//PropertyComponentShapeRotX,
		//PropertyComponentShapeRotY,
		//PropertyComponentShapeRotZ,
		PropertyMeshResPath,
		PropertyMeshInstance,
		PropertyMeshUseAnimation,
		PropertyMaterialList,
		PropertyMaterialShader,
		PropertyMaterialPassMask,
		PropertyMaterialCullMode,
		PropertyMaterialZEnable,
		PropertyMaterialZWriteEnable,
		PropertyMaterialBlendMode,
		PropertyMaterialMeshColor,
		PropertyMaterialMeshColorAlpha,
		PropertyMaterialMeshTexture,
		PropertyMaterialNormalTexture,
		PropertyMaterialSpecularTexture,
		PropertyClothSceneCollision,
		PropertyEmitterParticleList,
		PropertyEmitterParticleCount,
		PropertyEmitterParticlePosition,
		PropertyEmitterParticlePositionX,
		PropertyEmitterParticlePositionY,
		PropertyEmitterParticlePositionZ,
		PropertyEmitterParticleVelocity,
		PropertyEmitterParticleVelocityX,
		PropertyEmitterParticleVelocityY,
		PropertyEmitterParticleVelocityZ,
		PropertyEmitterParticleColor,
		PropertyEmitterParticleColorAlpha,
		PropertyEmitterParticleSize,
		PropertyEmitterParticleSizeX,
		PropertyEmitterParticleSizeY,
		PropertyEmitterParticleAngle,
		PropertySphericalEmitterParticleLifeTime,
		PropertySphericalEmitterSpawnInterval,
		PropertySphericalEmitterHalfSpawnArea,
		PropertySphericalEmitterHalfSpawnAreaX,
		PropertySphericalEmitterHalfSpawnAreaY,
		PropertySphericalEmitterHalfSpawnAreaZ,
		PropertySphericalEmitterSpawnSpeed,
		PropertySphericalEmitterSpawnInclination,
		PropertySphericalEmitterSpawnAzimuth,
		PropertySphericalEmitterSpawnColorR,
		PropertySphericalEmitterSpawnColorG,
		PropertySphericalEmitterSpawnColorB,
		PropertySphericalEmitterSpawnColorA,
		PropertySphericalEmitterSpawnSizeX,
		PropertySphericalEmitterSpawnSizeY,
		PropertySphericalEmitterSpawnAngle,
		PropertySphericalEmitterSpawnLoopTime,
		PropertySplineNodeCount,
		PropertySplineNodeX,
		PropertySplineNodeY,
		PropertySplineNodeK0,
		PropertySplineNodeK,
		PropertyTerrainRowChunks,
		PropertyTerrainColChunks,
		PropertyTerrainChunkSize,
		PropertyTerrainHeightScale,
		PropertyTerrainWrappedU,
		PropertyTerrainWrappedV,
		PropertyTerrainHeightMap,
		PropertyCount
	};
	CMFCPropertyGridProperty * m_pProp[PropertyCount];

	void OnSelectionChanged(EventArgs * arg);
	void OnCmpAttriChanged(EventArgs * arg);
	void RemovePropertiesFrom(CMFCPropertyGridProperty * pParentCtrl, int i);
	void UpdatePropertiesActor(Actor * actor);
	void UpdateProperties(CMFCPropertyGridProperty * pParentCtrl, int i, Component * cmp);
	void UpdatePropertiesMesh(CMFCPropertyGridProperty * pComponent, MeshComponent * mesh_cmp);
	void UpdatePropertiesMaterial(CMFCPropertyGridProperty * pParentCtrl, int NodeId, Material * mat);
	void UpdatePropertiesCloth(CMFCPropertyGridProperty * pComponent, ClothComponent * cloth_cmp);
	void UpdatePropertiesStaticEmitter(CMFCPropertyGridProperty * pComponent, EmitterComponent * emit_cmp);
	void UpdatePropertiesStaticEmitterParticle(CMFCPropertyGridProperty * pParentProp, int NodeId, EmitterComponent * emit_cmp);
	void UpdatePropertiesSphericalEmitter(CMFCPropertyGridProperty * pComponent, SphericalEmitterComponent * sphe_emit_cmp);
	void UpdatePropertiesSpline(CMFCPropertyGridProperty * pSpline, my::Spline * spline);
	void UpdatePropertiesSplineNode(CMFCPropertyGridProperty * pSpline, int NodeId, const my::SplineNode * node);
	void UpdatePropertiesTerrain(CMFCPropertyGridProperty * pComponent, Terrain * terrain);

	void CreatePropertiesActor(Actor * actor);
	void CreateProperties(CMFCPropertyGridProperty * pParentCtrl, int i, Component * cmp);
	void CreatePropertiesMesh(CMFCPropertyGridProperty * pComponent, MeshComponent * mesh_cmp);
	void CreatePropertiesMaterial(CMFCPropertyGridProperty * pParentCtrl, int NodeId, Material * mat);
	void CreatePropertiesCloth(CMFCPropertyGridProperty * pComponent, ClothComponent * cloth_cmp);
	void CreatePropertiesStaticEmitter(CMFCPropertyGridProperty * pComponent, EmitterComponent * emit_cmp);
	void CreatePropertiesStaticEmitterParticle(CMFCPropertyGridProperty * pParentProp, int NodeId, EmitterComponent * emit_cmp);
	void CreatePropertiesSphericalEmitter(CMFCPropertyGridProperty * pComponent, SphericalEmitterComponent * sphe_emit_cmp);
	void CreatePropertiesSpline(CMFCPropertyGridProperty * pParentProp, LPCTSTR lpszName, Property PropertyId, my::Spline * spline);
	void CreatePropertiesSplineNode(CMFCPropertyGridProperty * pSpline, int NodeId, my::SplineNode * node);
	void CreatePropertiesTerrain(CMFCPropertyGridProperty * pComponent, Terrain * terrain);

	static unsigned int GetComponentPropCount(Component::ComponentType type);
	static LPCTSTR GetComponentTypeName(Component::ComponentType type);

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

