// Copyright (c) 2011-2024 tangyin025
// License: MIT

#pragma once

class Actor;

class Material;

class MaterialParameter;

class Component;

class MeshComponent;

class StaticMesh;

class ClothComponent;

class EmitterComponent;

class StaticEmitter;

class SphericalEmitter;

class Terrain;

class TerrainChunk;

class Animator;

class AnimationNode;

class AnimationNodeSequence;

class Navigation;

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
	CPropertiesToolBar m_wndToolBar;
	CMFCPropertyGridCtrl m_wndPropList;
	BOOL m_OnPropertyChangeMuted;
public:
	struct PassMaskDesc
	{
		LPCTSTR desc;
		DWORD mask;
	};

	enum Property
	{
		PropertyUnknown = 0,
		PropertyActor,
		PropertyActorName,
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
		PropertyActorLodDist,
		PropertyActorLodFactor,
		PropertyActorCullingDistSq,
		PropertyActorRigidActor,
		PropertyActorRigidActorType,
		PropertyActorRigidActorKinematic,
		PropertyComponent,
		PropertyComponentName,
		PropertyComponentLODMask,
		PropertyComponentSiblingId,
		PropertyShape,
		PropertyShapeType,
		PropertyShapeLocalPos,
		PropertyShapeLocalPosX,
		PropertyShapeLocalPosY,
		PropertyShapeLocalPosZ,
		PropertyShapeLocalRot,
		PropertyShapeLocalRotX,
		PropertyShapeLocalRotY,
		PropertyShapeLocalRotZ,
		PropertyShapeSimulationFilterData,
		PropertyShapeQueryFilterData,
		PropertyShapeSimulation,
		PropertyShapeSceneQuery,
		PropertyShapeTrigger,
		PropertyShapeVisualization,
		PropertyMeshPath,
		PropertyMeshColor,
		PropertyMeshAlpha,
		PropertyMeshSubMeshId,
		PropertyMeshNumVert,
		PropertyMeshNumFace,
		PropertyMeshInstanceType,
		PropertyMaterial,
		PropertyMaterialShader,
		PropertyMaterialPassMask,
		PropertyMaterialCullMode,
		PropertyMaterialZEnable,
		PropertyMaterialZWriteEnable,
		PropertyMaterialZFunc,
		PropertyMaterialAlphaTestEnable,
		PropertyMaterialAlphaRef,
		PropertyMaterialAlphaFunc,
		PropertyMaterialBlendMode,
		PropertyMaterialParameterList,
		PropertyMaterialParameterInt2,
		PropertyMaterialParameterIntValueX,
		PropertyMaterialParameterIntValueY,
		PropertyMaterialParameterFloat,
		PropertyMaterialParameterFloat2,
		PropertyMaterialParameterFloat3,
		PropertyMaterialParameterFloat4,
		PropertyMaterialParameterFloatValueX,
		PropertyMaterialParameterFloatValueY,
		PropertyMaterialParameterFloatValueZ,
		PropertyMaterialParameterFloatValueW,
		PropertyMaterialParameterColor,
		PropertyMaterialParameterTexture,
		PropertyStaticMeshChunkWidth,
		PropertyStaticMeshChunkPath,
		PropertyStaticMeshChunkLodScale,
		PropertyClothColor,
		PropertyClothAlpha,
		PropertyClothSweptContact,
		PropertyClothSceneCollision,
		PropertyClothSolverFrequency,
		PropertyClothStiffnessFrequency,
		PropertyClothCollisionSpheresNum,
		PropertyClothCollisionCapsulesNum,
		PropertyClothVirtualParticleLevel,
		PropertyClothVirtualParticleNum,
		PropertyClothStretchVertical,
		PropertyClothStretchVerticalStiffness,
		PropertyClothStretchVerticalStiffnessMultiplier,
		PropertyClothStretchVerticalCompressionLimit,
		PropertyClothStretchVerticalStretchLimit,
		PropertyClothTether,
		PropertyClothTetherStiffness,
		PropertyClothTetherStretchLimit,
		PropertyClothExternalAcceleration,
		PropertyClothExternalAccelerationX,
		PropertyClothExternalAccelerationY,
		PropertyClothExternalAccelerationZ,
		PropertyEmitterFaceType,
		PropertyEmitterSpaceType,
		PropertyEmitterTiles,
		PropertyEmitterTilesX,
		PropertyEmitterTilesY,
		PropertyEmitterPrimitiveType,
		PropertyEmitterMeshPath,
		PropertyEmitterMeshSubMeshId,
		PropertyEmitterMeshNumVert,
		PropertyEmitterMeshNumFace,
		PropertyEmitterParticlePosition,
		PropertyEmitterParticlePositionX,
		PropertyEmitterParticlePositionY,
		PropertyEmitterParticlePositionZ,
		PropertyEmitterParticlePositionW,
		PropertyEmitterParticleVelocity,
		PropertyEmitterParticleVelocityX,
		PropertyEmitterParticleVelocityY,
		PropertyEmitterParticleVelocityZ,
		PropertyEmitterParticleVelocityW,
		PropertyEmitterParticleColor,
		PropertyEmitterParticleColorAlpha,
		PropertyEmitterParticleSize,
		PropertyEmitterParticleSizeX,
		PropertyEmitterParticleSizeY,
		PropertyEmitterParticleAngle,
		PropertyStaticEmitterChunkWidth,
		PropertyStaticEmitterChunkPath,
		PropertyStaticEmitterChunkLodScale,
		PropertySphericalEmitterParticleCapacity,
		PropertySphericalEmitterSpawnInterval,
		PropertySphericalEmitterSpawnCount,
		PropertySphericalEmitterHalfSpawnArea,
		PropertySphericalEmitterHalfSpawnAreaX,
		PropertySphericalEmitterHalfSpawnAreaY,
		PropertySphericalEmitterHalfSpawnAreaZ,
		PropertySphericalEmitterSpawnInclination,
		PropertySphericalEmitterSpawnInclinationX,
		PropertySphericalEmitterSpawnInclinationY,
		PropertySphericalEmitterSpawnAzimuth,
		PropertySphericalEmitterSpawnAzimuthX,
		PropertySphericalEmitterSpawnAzimuthY,
		PropertySphericalEmitterSpawnSpeed,
		PropertySphericalEmitterSpawnBoneId,
		PropertySphericalEmitterSpawnLocalPos,
		PropertySphericalEmitterSpawnLocalPosX,
		PropertySphericalEmitterSpawnLocalPosY,
		PropertySphericalEmitterSpawnLocalPosZ,
		PropertySphericalEmitterSpawnLocalRot,
		PropertySphericalEmitterSpawnLocalRotX,
		PropertySphericalEmitterSpawnLocalRotY,
		PropertySphericalEmitterSpawnLocalRotZ,
		PropertySphericalEmitterParticleLifeTime,
		PropertySphericalEmitterParticleGravity,
		PropertySphericalEmitterParticleGravityX,
		PropertySphericalEmitterParticleGravityY,
		PropertySphericalEmitterParticleGravityZ,
		PropertySphericalEmitterParticleDamping,
		PropertySphericalEmitterParticleColorR,
		PropertySphericalEmitterParticleColorG,
		PropertySphericalEmitterParticleColorB,
		PropertySphericalEmitterParticleColorA,
		PropertySphericalEmitterParticleSizeX,
		PropertySphericalEmitterParticleSizeY,
		PropertySphericalEmitterParticleAngle,
		PropertySplineNodeCount,
		PropertySplineNodeX,
		PropertySplineNodeY,
		PropertySplineNodeK0,
		PropertySplineNodeK,
		PropertyTerrainRowChunks,
		PropertyTerrainColChunks,
		PropertyTerrainChunkSize,
		PropertyTerrainChunkPath,
		PropertyTerrainChunkLodScale,
		PropertyTerrainHeightMap,
		PropertyTerrainSplatMap,
		PropertyAnimatorSkeletonPath,
		PropertyAnimationNode,
		PropertyAnimationNodeType,
		PropertyAnimationNodeSequenceName,
		PropertyAnimationNodeSequenceRate,
		PropertyNavigationNavMeshPath,
		PropertyNavigationOrigin,
		PropertyNavigationOriginX,
		PropertyNavigationOriginY,
		PropertyNavigationOriginZ,
		PropertyNavigationTileWidth,
		PropertyNavigationTileHeight,
		PropertyNavigationMaxTiles,
		PropertyNavigationMaxPolys,
		PropertyPaint,
		PropertyPaintShape,
		PropertyPaintMode,
		PropertyPaintRadius,
		PropertyPaintHeight,
		PropertyPaintColor,
		PropertyPaintAlpha,
		PropertyPaintSpline,
		PropertyPaintEmitterSiblingId,
		PropertyPaintParticleMinDist,
		PropertyPaintParticleAngle,
		PropertyPaintParticleAngleMin,
		PropertyPaintParticleAngleMax,
		PropertyControl,
		PropertyControlName,
		PropertyControlX,
		PropertyControlXScale,
		PropertyControlXOffset,
		PropertyControlY,
		PropertyControlYScale,
		PropertyControlYOffset,
		PropertyControlWidth,
		PropertyControlWidthScale,
		PropertyControlWidthOffset,
		PropertyControlHeight,
		PropertyControlHeightScale,
		PropertyControlHeightOffset,
		PropertyControlEnabled,
		PropertyControlVisible,
		PropertyControlFocused,
		PropertyControlSiblingId,
		PropertyControlColor,
		PropertyControlColorAlpha,
		PropertyControlImagePath,
		PropertyControlImageRect,
		PropertyControlImageRectLeft,
		PropertyControlImageRectTop,
		PropertyControlImageRectWidth,
		PropertyControlImageRectHeight,
		PropertyControlImageBorder,
		PropertyControlImageBorderX,
		PropertyControlImageBorderY,
		PropertyControlImageBorderZ,
		PropertyControlImageBorderW,
		PropertyControlVisibleShowSoundPath,
		PropertyControlVisibleHideSoundPath,
		PropertyControlMouseEnterSoundPath,
		PropertyControlMouseLeaveSoundPath,
		PropertyControlMouseClickSoundPath,
		PropertyStaticText,
		PropertyStaticFontPath,
		PropertyStaticFontHeight,
		PropertyStaticFontFaceIndex,
		PropertyStaticTextColor,
		PropertyStaticTextColorAlpha,
		PropertyStaticTextAlign,
		PropertyStaticTextOutlineColor,
		PropertyStaticTextOutlineAlpha,
		PropertyStaticTextOutlineWidth,
		PropertyProgressBarProgress,
		PropertyProgressBarForegroundImagePath,
		PropertyProgressBarForegroundImageRect,
		PropertyProgressBarForegroundImageRectLeft,
		PropertyProgressBarForegroundImageRectTop,
		PropertyProgressBarForegroundImageRectWidth,
		PropertyProgressBarForegroundImageRectHeight,
		PropertyProgressBarForegroundImageBorder,
		PropertyProgressBarForegroundImageBorderX,
		PropertyProgressBarForegroundImageBorderY,
		PropertyProgressBarForegroundImageBorderZ,
		PropertyProgressBarForegroundImageBorderW,
		PropertyButtonPressed,
		PropertyButtonMouseOver,
		PropertyButtonPressedOffset,
		PropertyButtonPressedOffsetX,
		PropertyButtonPressedOffsetY,
		PropertyButtonDisabledImagePath,
		PropertyButtonDisabledImageRect,
		PropertyButtonDisabledImageRectLeft,
		PropertyButtonDisabledImageRectTop,
		PropertyButtonDisabledImageRectWidth,
		PropertyButtonDisabledImageRectHeight,
		PropertyButtonDisabledImageBorder,
		PropertyButtonDisabledImageBorderX,
		PropertyButtonDisabledImageBorderY,
		PropertyButtonDisabledImageBorderZ,
		PropertyButtonDisabledImageBorderW,
		PropertyButtonPressedImagePath,
		PropertyButtonPressedImageRect,
		PropertyButtonPressedImageRectLeft,
		PropertyButtonPressedImageRectTop,
		PropertyButtonPressedImageRectWidth,
		PropertyButtonPressedImageRectHeight,
		PropertyButtonPressedImageBorder,
		PropertyButtonPressedImageBorderX,
		PropertyButtonPressedImageBorderY,
		PropertyButtonPressedImageBorderZ,
		PropertyButtonPressedImageBorderW,
		PropertyButtonMouseOverImagePath,
		PropertyButtonMouseOverImageRect,
		PropertyButtonMouseOverImageRectLeft,
		PropertyButtonMouseOverImageRectTop,
		PropertyButtonMouseOverImageRectWidth,
		PropertyButtonMouseOverImageRectHeight,
		PropertyButtonMouseOverImageBorder,
		PropertyButtonMouseOverImageBorderX,
		PropertyButtonMouseOverImageBorderY,
		PropertyButtonMouseOverImageBorderZ,
		PropertyButtonMouseOverImageBorderW,
		PropertyEditBoxBorder,
		PropertyEditBoxBorderX,
		PropertyEditBoxBorderY,
		PropertyEditBoxBorderZ,
		PropertyEditBoxBorderW,
		PropertyEditBoxDisabledImagePath,
		PropertyEditBoxDisabledImageRect,
		PropertyEditBoxDisabledImageRectLeft,
		PropertyEditBoxDisabledImageRectTop,
		PropertyEditBoxDisabledImageRectWidth,
		PropertyEditBoxDisabledImageRectHeight,
		PropertyEditBoxDisabledImageBorder,
		PropertyEditBoxDisabledImageBorderX,
		PropertyEditBoxDisabledImageBorderY,
		PropertyEditBoxDisabledImageBorderZ,
		PropertyEditBoxDisabledImageBorderW,
		PropertyEditBoxFocusedImagePath,
		PropertyEditBoxFocusedImageRect,
		PropertyEditBoxFocusedImageRectLeft,
		PropertyEditBoxFocusedImageRectTop,
		PropertyEditBoxFocusedImageRectWidth,
		PropertyEditBoxFocusedImageRectHeight,
		PropertyEditBoxFocusedImageBorder,
		PropertyEditBoxFocusedImageBorderX,
		PropertyEditBoxFocusedImageBorderY,
		PropertyEditBoxFocusedImageBorderZ,
		PropertyEditBoxFocusedImageBorderW,
		PropertyEditBoxSelBkColor,
		PropertyEditBoxSelBkColorAlpha,
		PropertyEditBoxCaretColor,
		PropertyEditBoxCaretColorAlpha,
		PropertyEditBoxCaretImagePath,
		PropertyEditBoxCaretImageRect,
		PropertyEditBoxCaretImageRectLeft,
		PropertyEditBoxCaretImageRectTop,
		PropertyEditBoxCaretImageRectWidth,
		PropertyEditBoxCaretImageRectHeight,
		PropertyEditBoxCaretImageBorder,
		PropertyEditBoxCaretImageBorderX,
		PropertyEditBoxCaretImageBorderY,
		PropertyEditBoxCaretImageBorderZ,
		PropertyEditBoxCaretImageBorderW,
		PropertyScrollBarUpDownButtonHeight,
		PropertyScrollBarPosition,
		PropertyScrollBarPageSize,
		PropertyScrollBarStart,
		PropertyScrollBarEnd,
		PropertyScrollBarPressedOffset,
		PropertyScrollBarPressedOffsetX,
		PropertyScrollBarPressedOffsetY,
		PropertyScrollBarUpBtnNormalImagePath,
		PropertyScrollBarUpBtnNormalImageRect,
		PropertyScrollBarUpBtnNormalImageRectLeft,
		PropertyScrollBarUpBtnNormalImageRectTop,
		PropertyScrollBarUpBtnNormalImageRectWidth,
		PropertyScrollBarUpBtnNormalImageRectHeight,
		PropertyScrollBarUpBtnNormalImageBorder,
		PropertyScrollBarUpBtnNormalImageBorderX,
		PropertyScrollBarUpBtnNormalImageBorderY,
		PropertyScrollBarUpBtnNormalImageBorderZ,
		PropertyScrollBarUpBtnNormalImageBorderW,
		PropertyScrollBarUpBtnDisabledImagePath,
		PropertyScrollBarUpBtnDisabledImageRect,
		PropertyScrollBarUpBtnDisabledImageRectLeft,
		PropertyScrollBarUpBtnDisabledImageRectTop,
		PropertyScrollBarUpBtnDisabledImageRectWidth,
		PropertyScrollBarUpBtnDisabledImageRectHeight,
		PropertyScrollBarUpBtnDisabledImageBorder,
		PropertyScrollBarUpBtnDisabledImageBorderX,
		PropertyScrollBarUpBtnDisabledImageBorderY,
		PropertyScrollBarUpBtnDisabledImageBorderZ,
		PropertyScrollBarUpBtnDisabledImageBorderW,
		PropertyScrollBarDownBtnNormalImagePath,
		PropertyScrollBarDownBtnNormalImageRect,
		PropertyScrollBarDownBtnNormalImageRectLeft,
		PropertyScrollBarDownBtnNormalImageRectTop,
		PropertyScrollBarDownBtnNormalImageRectWidth,
		PropertyScrollBarDownBtnNormalImageRectHeight,
		PropertyScrollBarDownBtnNormalImageBorder,
		PropertyScrollBarDownBtnNormalImageBorderX,
		PropertyScrollBarDownBtnNormalImageBorderY,
		PropertyScrollBarDownBtnNormalImageBorderZ,
		PropertyScrollBarDownBtnNormalImageBorderW,
		PropertyScrollBarDownBtnDisabledImagePath,
		PropertyScrollBarDownBtnDisabledImageRect,
		PropertyScrollBarDownBtnDisabledImageRectLeft,
		PropertyScrollBarDownBtnDisabledImageRectTop,
		PropertyScrollBarDownBtnDisabledImageRectWidth,
		PropertyScrollBarDownBtnDisabledImageRectHeight,
		PropertyScrollBarDownBtnDisabledImageBorder,
		PropertyScrollBarDownBtnDisabledImageBorderX,
		PropertyScrollBarDownBtnDisabledImageBorderY,
		PropertyScrollBarDownBtnDisabledImageBorderZ,
		PropertyScrollBarDownBtnDisabledImageBorderW,
		PropertyScrollBarThumbBtnNormalImagePath,
		PropertyScrollBarThumbBtnNormalImageRect,
		PropertyScrollBarThumbBtnNormalImageRectLeft,
		PropertyScrollBarThumbBtnNormalImageRectTop,
		PropertyScrollBarThumbBtnNormalImageRectWidth,
		PropertyScrollBarThumbBtnNormalImageRectHeight,
		PropertyScrollBarThumbBtnNormalImageBorder,
		PropertyScrollBarThumbBtnNormalImageBorderX,
		PropertyScrollBarThumbBtnNormalImageBorderY,
		PropertyScrollBarThumbBtnNormalImageBorderZ,
		PropertyScrollBarThumbBtnNormalImageBorderW,
		PropertyCheckBoxChecked,
		PropertyComboBoxDropdownSize,
		PropertyComboBoxDropdownSizeX,
		PropertyComboBoxDropdownSizeY,
		PropertyComboBoxScrollbarWidth,
		PropertyComboBoxScrollbarUpDownBtnHeight,
		PropertyComboBoxBorder,
		PropertyComboBoxBorderX,
		PropertyComboBoxBorderY,
		PropertyComboBoxBorderZ,
		PropertyComboBoxBorderW,
		PropertyComboBoxItemHeight,
		PropertyComboBoxItemCount,
		PropertyComboBoxDropdownImagePath,
		PropertyComboBoxDropdownImageRect,
		PropertyComboBoxDropdownImageRectLeft,
		PropertyComboBoxDropdownImageRectTop,
		PropertyComboBoxDropdownImageRectWidth,
		PropertyComboBoxDropdownImageRectHeight,
		PropertyComboBoxDropdownImageBorder,
		PropertyComboBoxDropdownImageBorderX,
		PropertyComboBoxDropdownImageBorderY,
		PropertyComboBoxDropdownImageBorderZ,
		PropertyComboBoxDropdownImageBorderW,
		PropertyComboBoxDropdownItemTextColor,
		PropertyComboBoxDropdownItemTextColorAlpha,
		PropertyComboBoxDropdownItemTextAlign,
		PropertyComboBoxDropdownItemMouseOverImagePath,
		PropertyComboBoxDropdownItemMouseOverImageRect,
		PropertyComboBoxDropdownItemMouseOverImageRectLeft,
		PropertyComboBoxDropdownItemMouseOverImageRectTop,
		PropertyComboBoxDropdownItemMouseOverImageRectWidth,
		PropertyComboBoxDropdownItemMouseOverImageRectHeight,
		PropertyComboBoxDropdownItemMouseOverImageBorder,
		PropertyComboBoxDropdownItemMouseOverImageBorderX,
		PropertyComboBoxDropdownItemMouseOverImageBorderY,
		PropertyComboBoxDropdownItemMouseOverImageBorderZ,
		PropertyComboBoxDropdownItemMouseOverImageBorderW,
		PropertyListBoxScrollbarWidth,
		PropertyListBoxScrollbarUpDownBtnHeight,
		PropertyListBoxItemSize,
		PropertyListBoxItemSizeX,
		PropertyListBoxItemSizeY,
		PropertyListBoxItemCount,
		PropertyDialogEnableDrag,
		PropertyDialogPos,
		PropertyDialogPosX,
		PropertyDialogPosY,
		PropertyDialogPosZ,
		PropertyDialogRot,
		PropertyDialogRotX,
		PropertyDialogRotY,
		PropertyDialogRotZ,
		PropertyDialogScale,
		PropertyDialogScaleX,
		PropertyDialogScaleY,
		PropertyDialogScaleZ,
		PropertyCount
	};
	CMFCPropertyGridProperty * m_pProp[PropertyCount];

	void OnSelectionChanged(my::EventArg * arg);
	static void RemovePropertiesFrom(CMFCPropertyGridProperty * pParentCtrl, int i);
	void UpdatePropertiesActor(Actor * actor);
	void UpdatePropertiesRigidActor(CMFCPropertyGridProperty * pRigidActor, Actor * actor);
	void UpdateProperties(CMFCPropertyGridProperty * pComponent, int i, Component * cmp);
	void UpdatePropertiesShape(CMFCPropertyGridProperty * pShape, Component * cmp);
	void UpdatePropertiesShapeShow(CMFCPropertyGridProperty * pShape, BOOL bShow);
	void UpdatePropertiesMesh(CMFCPropertyGridProperty * pComponent, MeshComponent * mesh_cmp);
	static void UpdatePropertiesMaterial(CMFCPropertyGridProperty * pMaterial, Material * mtl);
	static void UpdatePropertiesMaterialParameter(CMFCPropertyGridProperty * pParentCtrl, int NodeId, MaterialParameter * mtl_param);
	void UpdatePropertiesStaticMesh(CMFCPropertyGridProperty * pComponent, StaticMesh * static_mesh_cmp);
	void UpdatePropertiesCloth(CMFCPropertyGridProperty * pComponent, ClothComponent * cloth_cmp);
	void UpdatePropertiesEmitter(CMFCPropertyGridProperty * pComponent, EmitterComponent * emit_cmp);
	void UpdatePropertiesStaticEmitter(CMFCPropertyGridProperty * pComponent, StaticEmitter * emit_cmp);
	void UpdatePropertiesStaticEmitterParticle(CMFCPropertyGridProperty * pParticle, const CPoint & chunkid, int instid, my::Emitter::Particle * particle);
	void UpdatePropertiesSphericalEmitter(CMFCPropertyGridProperty * pComponent, SphericalEmitter * sphe_emit_cmp);
	void UpdatePropertiesSpline(CMFCPropertyGridProperty * pSpline, my::Spline * spline);
	void UpdatePropertiesSplineNode(CMFCPropertyGridProperty * pSpline, int NodeId, const my::Spline::value_type* node);
	void UpdatePropertiesTerrain(CMFCPropertyGridProperty * pComponent, Terrain * terrain);
	void UpdatePropertiesAnimator(CMFCPropertyGridProperty * pComponent, Animator * animator);
	void UpdatePropertiesAnimationNode(CMFCPropertyGridProperty * pAnimationNode, AnimationNode * node);
	void UpdatePropertiesAnimationNodeSequence(CMFCPropertyGridProperty * pAnimationNode, AnimationNodeSequence * seq);
	void UpdatePropertiesNavigation(CMFCPropertyGridProperty * pComponent, Navigation * navigation);
	void UpdatePropertiesControl(my::Control * control);
	void UpdatePropertiesStatic(CMFCPropertyGridProperty * pControl, my::Static * static_ctl);
	void UpdatePropertiesProgressBar(CMFCPropertyGridProperty * pControl, my::ProgressBar * progressbar);
	void UpdatePropertiesButton(CMFCPropertyGridProperty * pControl, my::Button * button);
	void UpdatePropertiesEditBox(CMFCPropertyGridProperty * pControl, my::EditBox * editbox);
	void UpdatePropertiesScrollBar(CMFCPropertyGridProperty * pControl, my::ScrollBar * scrollbar);
	void UpdatePropertiesCheckBox(CMFCPropertyGridProperty * pControl, my::CheckBox * checkbox);
	void UpdatePropertiesComboBox(CMFCPropertyGridProperty * pControl, my::ComboBox * combobox);
	void UpdatePropertiesListBox(CMFCPropertyGridProperty * pControl, my::ListBox * listbox);
	void UpdatePropertiesDialog(CMFCPropertyGridProperty * pControl, my::Dialog * dialog);

	void CreatePropertiesActor(Actor * actor);
	void CreatePropertiesRigidActor(CMFCPropertyGridProperty * pParentCtrl, Actor * actor);
	void CreateProperties(CMFCPropertyGridProperty * pParentCtrl, Component * cmp);
	void CreatePropertiesShape(CMFCPropertyGridProperty * pParentCtrl, Component * cmp);
	void CreatePropertiesMesh(CMFCPropertyGridProperty * pComponent, MeshComponent * mesh_cmp);
	static void CreatePropertiesMaterial(CMFCPropertyGridProperty * pParentCtrl, LPCTSTR lpszName, Material * mtl);
	static void CreatePropertiesMaterialParameter(CMFCPropertyGridProperty * pParentCtrl, int NodeId, MaterialParameter * mtl_param);
	void CreatePropertiesStaticMesh(CMFCPropertyGridProperty * pComponent, StaticMesh * static_mesh_cmp);
	void CreatePropertiesCloth(CMFCPropertyGridProperty * pComponent, ClothComponent * cloth_cmp);
	void CreatePropertiesEmitter(CMFCPropertyGridProperty * pComponent, EmitterComponent * emit_cmp);
	void CreatePropertiesStaticEmitter(CMFCPropertyGridProperty * pComponent, StaticEmitter * emit_cmp);
	void CreatePropertiesStaticEmitterParticle(CMFCPropertyGridProperty * pParentProp, const CPoint & chunkid, int instid, my::Emitter::Particle * particle);
	void CreatePropertiesSphericalEmitter(CMFCPropertyGridProperty * pComponent, SphericalEmitter * sphe_emit_cmp);
	void CreatePropertiesSpline(CMFCPropertyGridProperty * pParentProp, LPCTSTR lpszName, Property PropertyId, my::Spline * spline);
	void CreatePropertiesSplineNode(CMFCPropertyGridProperty * pSpline, int NodeId, my::Spline::value_type * node);
	void CreatePropertiesTerrain(CMFCPropertyGridProperty * pComponent, Terrain * terrain);
	void CreatePropertiesAnimator(CMFCPropertyGridProperty * pComponent, Animator * animator);
	void CreatePropertiesAnimationNode(CMFCPropertyGridProperty * pParentCtrl, AnimationNode * node);
	void CreatePropertiesAnimationNodeSequence(CMFCPropertyGridProperty * pAnimationNode, AnimationNodeSequence * seq);
	void CreatePropertiesNavigation(CMFCPropertyGridProperty * pComponent, Navigation * navigation);
	void CreatePropertiesControl(my::Control * control);
	void CreatePropertiesStatic(CMFCPropertyGridProperty * pControl, my::Static * static_ctl);
	void CreatePropertiesProgressBar(CMFCPropertyGridProperty * pControl, my::ProgressBar * progressbar);
	void CreatePropertiesButton(CMFCPropertyGridProperty * pControl, my::Button * button);
	void CreatePropertiesEditBox(CMFCPropertyGridProperty * pControl, my::EditBox * editbox);
	void CreatePropertiesScrollBar(CMFCPropertyGridProperty * pControl, my::ScrollBar * scrollbar);
	void CreatePropertiesCheckBox(CMFCPropertyGridProperty * pControl, my::CheckBox * checkbox);
	void CreatePropertiesComboBox(CMFCPropertyGridProperty * pControl, my::ComboBox * combobox);
	void CreatePropertiesListBox(CMFCPropertyGridProperty * pControl, my::ListBox * listbox);
	void CreatePropertiesDialog(CMFCPropertyGridProperty * pControl, my::Dialog * dialog);

	static unsigned int GetComponentPropCount(DWORD type);
	static LPCTSTR GetComponentTypeName(DWORD type);
	static TerrainChunk * GetTerrainChunkSafe(Terrain * terrain, const CPoint & chunkid);
	static Property GetMaterialParameterTypeProp(MaterialParameter * mtl_param);
	static unsigned int GetControlPropCount(DWORD type);
	static LPCTSTR GetControlTypeName(DWORD type);

	void UpdatePropertiesPaintTool(Actor* actor);
	void CreatePropertiesPaintTool(Actor* actor);

// Implementation
public:
	virtual ~CPropertiesWnd();

protected:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnDestroy();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnExpandAllProperties();
	afx_msg void OnUpdateExpandAllProperties(CCmdUI* pCmdUI);
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

