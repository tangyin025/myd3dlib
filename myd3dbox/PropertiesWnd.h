
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
		PropertyComponent,
		PropertyComponentPos,
		PropertyComponentPosX,
		PropertyComponentPosY,
		PropertyComponentPosZ,
		PropertyComponentRot,
		PropertyComponentRotX,
		PropertyComponentRotY,
		PropertyComponentRotZ,
		PropertyComponentScale,
		PropertyComponentScaleX,
		PropertyComponentScaleY,
		PropertyComponentScaleZ,
		PropertyActorAABB,
		PropertyActorMinX,
		PropertyActorMinY,
		PropertyActorMinZ,
		PropertyActorMaxX,
		PropertyActorMaxY,
		PropertyActorMaxZ,
		PropertyMeshResPath,
		PropertyMeshInstance,
		PropertyMeshUseAnimation,
		PropertyMeshStaticCollision,
		PropertyMaterialList,
		PropertyMaterialShader,
		PropertyMaterialPassMask,
		PropertyMaterialMeshColor,
		PropertyMaterialMeshColorR,
		PropertyMaterialMeshColorG,
		PropertyMaterialMeshColorB,
		PropertyMaterialMeshColorA,
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
		PropertyEmitterParticleColorR,
		PropertyEmitterParticleColorG,
		PropertyEmitterParticleColorB,
		PropertyEmitterParticleColorA,
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
		PropertyTerrainChunkRows,
		PropertyTerrainHeightScale,
		PropertyTerrainWrappedU,
		PropertyTerrainWrappedV,
		PropertyTerrainHeightMap,
		PropertyTerrainStaticCollision,
		//PropertyRigidShapeList,
		//PropertyRigidShapeAdd,
		//PropertyRigidShapePos,
		//PropertyRigidShapePosX,
		//PropertyRigidShapePosY,
		//PropertyRigidShapePosZ,
		//PropertyRigidShapeRot,
		//PropertyRigidShapeRotX,
		//PropertyRigidShapeRotY,
		//PropertyRigidShapeRotZ,
		//PropertyRigidShapeCapsuleRadius,
		//PropertyRigidShapeCapsuleHalfHeight,
		//PropertyRigidShapeBoxHalfExtents,
		//PropertyRigidShapeBoxHalfExtentsX,
		//PropertyRigidShapeBoxHalfExtentsY,
		//PropertyRigidShapeBoxHalfExtentsZ,
		//PropertyRigidShapeSphereRadius,
		PropertyCount
	};
	CMFCPropertyGridProperty * m_pProp[PropertyCount];

	void OnSelectionChanged(EventArg * arg);
	void OnCmpAttriChanged(EventArg * arg);
	void RemovePropertiesFrom(CMFCPropertyGridProperty * pParentCtrl, int i);
	void UpdateProperties(CMFCPropertyGridProperty * pParentCtrl, int i, Component * cmp);
	void UpdatePropertiesActor(CMFCPropertyGridProperty * pComponent, Actor * actor);
	void UpdatePropertiesCharacter(CMFCPropertyGridProperty * pComponent, Character * character);
	void UpdatePropertiesMesh(CMFCPropertyGridProperty * pComponent, MeshComponent * mesh_cmp);
	void UpdatePropertiesMaterial(CMFCPropertyGridProperty * pParentCtrl, int NodeId, Material * mat);
	void UpdatePropertiesCloth(CMFCPropertyGridProperty * pComponent, ClothComponent * cloth_cmp);
	void UpdatePropertiesStaticEmitter(CMFCPropertyGridProperty * pComponent, EmitterComponent * emit_cmp);
	void UpdatePropertiesStaticEmitterParticle(CMFCPropertyGridProperty * pParentProp, int NodeId, EmitterComponent * emit_cmp);
	void UpdatePropertiesSphericalEmitter(CMFCPropertyGridProperty * pComponent, SphericalEmitterComponent * sphe_emit_cmp);
	void UpdatePropertiesSpline(CMFCPropertyGridProperty * pSpline, my::Spline * spline);
	void UpdatePropertiesSplineNode(CMFCPropertyGridProperty * pSpline, int NodeId, const my::SplineNode * node);
	void UpdatePropertiesTerrain(CMFCPropertyGridProperty * pComponent, Terrain * terrain);
	//void UpdatePropertiesRigid(RigidComponent * cmp);
	//void UpdatePropertiesShape(CMFCPropertyGridProperty * pParentCtrl, int NodeId, PxShape * shape);
	//void UpdatePropertiesShapeBox(CMFCPropertyGridProperty * pShape, PxBoxGeometry & box);
	//void UpdatePropertiesShapeSphere(CMFCPropertyGridProperty * pShape, PxSphereGeometry & sphere);
	//void UpdatePropertiesShapeCapsule(CMFCPropertyGridProperty * pShape, PxCapsuleGeometry & capsule);

	void CreateProperties(CMFCPropertyGridProperty * pParentCtrl, int i, Component * cmp);
	void CreatePropertiesActor(CMFCPropertyGridProperty * pComponent, Actor * actor);
	void CreatePropertiesCharacter(CMFCPropertyGridProperty * pComponent, Character * character);
	void CreatePropertiesMesh(CMFCPropertyGridProperty * pComponent, MeshComponent * mesh_cmp);
	void CreatePropertiesMaterial(CMFCPropertyGridProperty * pParentCtrl, int NodeId, Material * mat);
	void CreatePropertiesCloth(CMFCPropertyGridProperty * pComponent, ClothComponent * cloth_cmp);
	void CreatePropertiesStaticEmitter(CMFCPropertyGridProperty * pComponent, EmitterComponent * emit_cmp);
	void CreatePropertiesStaticEmitterParticle(CMFCPropertyGridProperty * pParentProp, int NodeId, EmitterComponent * emit_cmp);
	void CreatePropertiesSphericalEmitter(CMFCPropertyGridProperty * pComponent, SphericalEmitterComponent * sphe_emit_cmp);
	void CreatePropertiesSpline(CMFCPropertyGridProperty * pParentProp, LPCTSTR lpszName, Property PropertyId, my::Spline * spline);
	void CreatePropertiesSplineNode(CMFCPropertyGridProperty * pSpline, int NodeId, my::SplineNode * node);
	void CreatePropertiesTerrain(CMFCPropertyGridProperty * pComponent, Terrain * terrain);
	//void CreatePropertiesShape(CMFCPropertyGridProperty * pParentCtrl, int NodeId, PxGeometryType::Enum type);
	//void CreatePropertiesShapeBox(CMFCPropertyGridProperty * pShape);
	//void CreatePropertiesShapeSphere(CMFCPropertyGridProperty * pShape);
	//void CreatePropertiesShapeCapsule(CMFCPropertyGridProperty * pShape);

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

