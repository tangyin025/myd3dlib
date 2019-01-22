
#pragma once

#include "EventDefine.h"

class Actor;

class Component;

class MeshComponent;

class ClothComponent;

class EmitterComponent;

class Material;

class MaterialParameter;

class SphericalEmitterComponent;

class Terrain;

class TerrainChunk;

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
		PropertyActorLodRatio,
		PropertyActorRigidActor,
		PropertyComponentLODMask,
		PropertyComponentShape,
		PropertyCharacter,
		PropertyMesh,
		PropertyMeshResPath,
		PropertyMeshInstance,
		PropertyMeshUseAnimation,
		PropertyMeshNavigation,
		PropertyMaterialList,
		PropertyMaterial,
		PropertyMaterialShader,
		PropertyMaterialPassMask,
		PropertyMaterialCullMode,
		PropertyMaterialZEnable,
		PropertyMaterialZWriteEnable,
		PropertyMaterialBlendMode,
		PropertyMaterialParameterList,
		PropertyMaterialParameterFloat,
		PropertyMaterialParameterFloat2,
		PropertyMaterialParameterFloat3,
		PropertyMaterialParameterFloat4,
		PropertyMaterialParameterFloatValueX,
		PropertyMaterialParameterFloatValueY,
		PropertyMaterialParameterFloatValueZ,
		PropertyMaterialParameterFloatValueW,
		PropertyMaterialParameterTexture,
		PropertyCloth,
		PropertyClothSceneCollision,
		PropertyStaticEmitter,
		PropertyEmitterToWorld,
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
		PropertySphericalEmitter,
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
		PropertyTerrain,
		PropertyTerrainRowChunks,
		PropertyTerrainColChunks,
		PropertyTerrainChunkSize,
		PropertyTerrainHeightScale,
		PropertyTerrainNavigation,
		PropertyTerrainHeightMap,
		PropertyTerrainGrassDensity,
		PropertyTerrainGrassStageRadius,
		PropertyTerrainGrassSize,
		PropertyTerrainGrassSizeX,
		PropertyTerrainGrassSizeY,
		PropertyCount
	};
	CMFCPropertyGridProperty * m_pProp[PropertyCount];

	void OnSelectionChanged(EventArgs * arg);
	void OnCmpAttriChanged(EventArgs * arg);
	void RemovePropertiesFrom(CMFCPropertyGridProperty * pParentCtrl, int i);
	void UpdatePropertiesActor(Actor * actor);
	void UpdateProperties(CMFCPropertyGridProperty * pComponent, int i, Component * cmp);
	void UpdatePropertiesMesh(CMFCPropertyGridProperty * pComponent, MeshComponent * mesh_cmp);
	void UpdatePropertiesMaterial(CMFCPropertyGridProperty * pMaterial, Material * mtl);
	void UpdatePropertiesMaterialParameter(CMFCPropertyGridProperty * pParentCtrl, int NodeId, MaterialParameter * mtl_param);
	void UpdatePropertiesCloth(CMFCPropertyGridProperty * pComponent, ClothComponent * cloth_cmp);
	void UpdatePropertiesStaticEmitter(CMFCPropertyGridProperty * pComponent, EmitterComponent * emit_cmp);
	void UpdatePropertiesStaticEmitterParticle(CMFCPropertyGridProperty * pParentProp, int NodeId, EmitterComponent * emit_cmp);
	void UpdatePropertiesSphericalEmitter(CMFCPropertyGridProperty * pComponent, SphericalEmitterComponent * sphe_emit_cmp);
	void UpdatePropertiesSpline(CMFCPropertyGridProperty * pSpline, my::Spline * spline);
	void UpdatePropertiesSplineNode(CMFCPropertyGridProperty * pSpline, int NodeId, const my::SplineNode * node);
	void UpdatePropertiesTerrain(CMFCPropertyGridProperty * pComponent, Terrain * terrain, const CPoint & chunkid);

	void CreatePropertiesActor(Actor * actor);
	void CreateProperties(CMFCPropertyGridProperty * pParentCtrl, Component * cmp);
	void CreatePropertiesMesh(CMFCPropertyGridProperty * pComponent, MeshComponent * mesh_cmp);
	void CreatePropertiesMaterial(CMFCPropertyGridProperty * pParentCtrl, LPCTSTR lpszName, Material * mtl);
	void CreatePropertiesMaterialParameter(CMFCPropertyGridProperty * pParentCtrl, int NodeId, MaterialParameter * mtl_param);
	void CreatePropertiesCloth(CMFCPropertyGridProperty * pComponent, ClothComponent * cloth_cmp);
	void CreatePropertiesStaticEmitter(CMFCPropertyGridProperty * pComponent, EmitterComponent * emit_cmp);
	void CreatePropertiesStaticEmitterParticle(CMFCPropertyGridProperty * pParentProp, int NodeId, EmitterComponent * emit_cmp);
	void CreatePropertiesSphericalEmitter(CMFCPropertyGridProperty * pComponent, SphericalEmitterComponent * sphe_emit_cmp);
	void CreatePropertiesSpline(CMFCPropertyGridProperty * pParentProp, LPCTSTR lpszName, Property PropertyId, my::Spline * spline);
	void CreatePropertiesSplineNode(CMFCPropertyGridProperty * pSpline, int NodeId, my::SplineNode * node);
	void CreatePropertiesTerrain(CMFCPropertyGridProperty * pComponent, Terrain * terrain, const CPoint & chunkid);

	static Property GetComponentProp(DWORD type);
	static unsigned int GetComponentPropCount(DWORD type);
	static LPCTSTR GetComponentTypeName(DWORD type);
	static TerrainChunk * GetTerrainChunkSafe(Terrain * terrain, const CPoint & chunkid);
	static Property GetMaterialParameterTypeProp(DWORD type);

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

