
#pragma once

#include "EventDefine.h"
#include "Component/Component.h"
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
		PropertyComponent,
		PropertyComponentAABB,
		PropertyComponentMinX,
		PropertyComponentMinY,
		PropertyComponentMinZ,
		PropertyComponentMaxX,
		PropertyComponentMaxY,
		PropertyComponentMaxZ,
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
		//PropertyMesh,
		//PropertyMeshLodList,
		//PropertyMeshLodCount,
		//PropertyMeshLodResPath,
		//PropertyMeshLodInstance,
		//PropertyMeshLodMaxDistance,
		//PropertyMeshLodBand,
		//PropertyMeshStaticCollision,
		//PropertyEmitter,
		//PropertyEmitterParticleList,
		//PropertyEmitterParticleCount,
		//PropertyEmitterParticlePosition,
		//PropertyEmitterParticlePositionX,
		//PropertyEmitterParticlePositionY,
		//PropertyEmitterParticlePositionZ,
		//PropertyEmitterParticleVelocity,
		//PropertyEmitterParticleVelocityX,
		//PropertyEmitterParticleVelocityY,
		//PropertyEmitterParticleVelocityZ,
		//PropertyEmitterParticleColor,
		//PropertyEmitterParticleColorR,
		//PropertyEmitterParticleColorG,
		//PropertyEmitterParticleColorB,
		//PropertyEmitterParticleColorA,
		//PropertyEmitterParticleSize,
		//PropertyEmitterParticleSizeX,
		//PropertyEmitterParticleSizeY,
		//PropertyEmitterParticleAngle,
		//PropertyDynamicEmitterParticleLifeTime,
		//PropertySphericalEmitterSpawnInterval,
		//PropertySphericalEmitterHalfSpawnArea,
		//PropertySphericalEmitterHalfSpawnAreaX,
		//PropertySphericalEmitterHalfSpawnAreaY,
		//PropertySphericalEmitterHalfSpawnAreaZ,
		//PropertySphericalEmitterSpawnSpeed,
		//PropertySphericalEmitterSpawnInclination,
		//PropertySphericalEmitterSpawnAzimuth,
		//PropertySphericalEmitterSpawnColorR,
		//PropertySphericalEmitterSpawnColorG,
		//PropertySphericalEmitterSpawnColorB,
		//PropertySphericalEmitterSpawnColorA,
		//PropertySphericalEmitterSpawnSizeX,
		//PropertySphericalEmitterSpawnSizeY,
		//PropertySphericalEmitterSpawnAngle,
		//PropertySphericalEmitterSpawnLoopTime,
		//PropertySplineNodeCount,
		//PropertySplineNodeX,
		//PropertySplineNodeY,
		//PropertySplineNodeK0,
		//PropertySplineNodeK,
		//PropertyMaterialList,
		//PropertyMaterialShader,
		//PropertyMaterialPassMask,
		//PropertyMaterialMeshColor,
		//PropertyMaterialMeshColorR,
		//PropertyMaterialMeshColorG,
		//PropertyMaterialMeshColorB,
		//PropertyMaterialMeshColorA,
		//PropertyMaterialMeshTexture,
		//PropertyMaterialNormalTexture,
		//PropertyMaterialSpecularTexture,
		////PropertyRigidShapeList,
		////PropertyRigidShapeAdd,
		////PropertyRigidShapePos,
		////PropertyRigidShapePosX,
		////PropertyRigidShapePosY,
		////PropertyRigidShapePosZ,
		////PropertyRigidShapeRot,
		////PropertyRigidShapeRotX,
		////PropertyRigidShapeRotY,
		////PropertyRigidShapeRotZ,
		////PropertyRigidShapeCapsuleRadius,
		////PropertyRigidShapeCapsuleHalfHeight,
		////PropertyRigidShapeBoxHalfExtents,
		////PropertyRigidShapeBoxHalfExtentsX,
		////PropertyRigidShapeBoxHalfExtentsY,
		////PropertyRigidShapeBoxHalfExtentsZ,
		////PropertyRigidShapeSphereRadius,
		//PropertyTerrain,
		//PropertyTerrainRowChunks,
		//PropertyTerrainChunkRows,
		//PropertyTerrainHeightScale,
		//PropertyTerrainWrappedU,
		//PropertyTerrainWrappedV,
		//PropertyTerrainHeightMap,
		//PropertyTerrainStaticCollision,
		PropertyCount
	};
	CMFCPropertyGridProperty * m_pProp[PropertyCount];

	void OnSelectionChanged(EventArg * arg);
	void OnCmpAttriChanged(EventArg * arg);
	void HideAllProperties(void);
	void RemovePropertiesFrom(CMFCPropertyGridProperty * pParentCtrl, DWORD i);
	//void UpdateProperties(Component * cmp);
	//void UpdatePropertiesMesh(MeshComponent * cmp);
	//void UpdatePropertiesMeshLodList(CMFCPropertyGridProperty * pLodList, MeshComponent * cmp);
	//void UpdatePropertiesMeshLod(CMFCPropertyGridProperty * pParentCtrl, DWORD NodeId, MeshComponent::LOD & lod);
	//void UpdatePropertiesEmitter(EmitterComponent * cmp);
	////void UpdatePropertiesRigid(RigidComponent * cmp);
	//void UpdatePropertiesTerrain(Terrain * terrain);
	//void UpdatePropertiesEmitterParticleList(CMFCPropertyGridProperty * pParticleList, const my::Emitter::ParticleList & particle_list);
	//void UpdatePropertiesEmitterParticle(CMFCPropertyGridProperty * pParentProp, DWORD NodeId, const my::Emitter::Particle & particle);
	//void UpdatePropertiesSpline(Property PropertyId, my::Spline * spline);
	//void UpdatePropertiesSplineNode(CMFCPropertyGridProperty * pSpline, DWORD NodeId, const my::SplineNode * node);
	//void UpdatePropertiesMaterial(CMFCPropertyGridProperty * pParentCtrl, DWORD NodeId, Material * mat);
	//void UpdatePropertiesShape(CMFCPropertyGridProperty * pParentCtrl, DWORD NodeId, PxShape * shape);
	//void UpdatePropertiesShapeBox(CMFCPropertyGridProperty * pShape, PxBoxGeometry & box);
	//void UpdatePropertiesShapeSphere(CMFCPropertyGridProperty * pShape, PxSphereGeometry & sphere);
	//void UpdatePropertiesShapeCapsule(CMFCPropertyGridProperty * pShape, PxCapsuleGeometry & capsule);

	void CreateProperties(CMFCPropertyGridProperty * pParentCtrl, Component * cmp);
	//void CreatePropertiesMeshLod(CMFCPropertyGridProperty * pParentCtrl, DWORD NodeId);
	//void CreatePropertiesEmitterParticle(CMFCPropertyGridProperty * pParentProp, DWORD NodeId);
	//void CreatePropertiesSpline(CMFCPropertyGridProperty * pParentProp, LPCTSTR lpszName, Property PropertyId);
	//void CreatePropertiesSplineNode(CMFCPropertyGridProperty * pSpline, DWORD NodeId);
	//void CreatePropertiesMaterial(CMFCPropertyGridProperty * pParentCtrl, DWORD NodeId);
	////void CreatePropertiesShape(CMFCPropertyGridProperty * pParentCtrl, DWORD NodeId, PxGeometryType::Enum type);
	////void CreatePropertiesShapeBox(CMFCPropertyGridProperty * pShape);
	////void CreatePropertiesShapeSphere(CMFCPropertyGridProperty * pShape);
	////void CreatePropertiesShapeCapsule(CMFCPropertyGridProperty * pShape);

	//Material * GetComponentMaterial(Component * cmp, unsigned int id);

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

