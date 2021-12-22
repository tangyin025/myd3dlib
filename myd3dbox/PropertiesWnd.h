
#pragma once

class Actor;

class Material;

class MaterialParameter;

class Component;

class MeshComponent;

class ClothComponent;

class StaticEmitter;

class SphericalEmitter;

class Terrain;

class TerrainChunk;

class Animator;

class AnimationNode;

class AnimationNodeSequence;

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
		PropertyActorRigidActor,
		PropertyActorRigidActorType,
		PropertyActorRigidActorKinematic,
		PropertyComponentName,
		PropertyComponentLODMask,
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
		PropertyCharacter,
		PropertyMesh,
		PropertyMeshPath,
		PropertyMeshSubMeshName,
		PropertyMeshSubMeshId,
		PropertyMeshColor,
		PropertyMeshAlpha,
		PropertyMeshInstance,
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
		PropertyClothColor,
		PropertyClothAlpha,
		PropertyClothSceneCollision,
		PropertyStaticEmitter,
		PropertyEmitterFaceType,
		PropertyEmitterSpaceType,
		PropertyEmitterVelType,
		PropertyEmitterPrimitiveType,
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
		PropertySphericalEmitter,
		PropertySphericalEmitterParticleCapacity,
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
		PropertySphericalEmitterSpawnCycle,
		PropertySplineNodeCount,
		PropertySplineNodeX,
		PropertySplineNodeY,
		PropertySplineNodeK0,
		PropertySplineNodeK,
		PropertyTerrain,
		PropertyTerrainRowChunks,
		PropertyTerrainColChunks,
		PropertyTerrainChunkSize,
		PropertyTerrainChunkPath,
		PropertyTerrainHeightMap,
		PropertyTerrainSplatMap,
		PropertyTerrainChunkMaterial,
		PropertyAnimator,
		PropertyAnimatorSkeletonPath,
		PropertyAnimationNode,
		PropertyAnimationNodeType,
		PropertyAnimationNodeSequenceName,
		PropertyNavigation,
		PropertyPaint,
		PropertyPaintShape,
		PropertyPaintMode,
		PropertyPaintRadius,
		PropertyPaintHeight,
		PropertyPaintColor,
		PropertyPaintSpline,
		PropertyPaintParticleMinDist,
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
		PropertyControlFocused,
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
		PropertyControlFontPath,
		PropertyControlFontHeight,
		PropertyControlFontFaceIndex,
		PropertyControlTextColor,
		PropertyControlTextColorAlpha,
		PropertyControlTextAlign,
		PropertyControlTextOutlineColor,
		PropertyControlTextOutlineAlpha,
		PropertyControlTextOutlineWidth,
		PropertyStaticText,
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
		PropertyButtonPressedOffset,
		PropertyButtonPressedOffsetX,
		PropertyButtonPressedOffsetY,
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
		PropertyComboBoxScrollBarUpBtnNormalImagePath,
		PropertyComboBoxScrollBarUpBtnNormalImageRect,
		PropertyComboBoxScrollBarUpBtnNormalImageRectLeft,
		PropertyComboBoxScrollBarUpBtnNormalImageRectTop,
		PropertyComboBoxScrollBarUpBtnNormalImageRectWidth,
		PropertyComboBoxScrollBarUpBtnNormalImageRectHeight,
		PropertyComboBoxScrollBarUpBtnNormalImageBorder,
		PropertyComboBoxScrollBarUpBtnNormalImageBorderX,
		PropertyComboBoxScrollBarUpBtnNormalImageBorderY,
		PropertyComboBoxScrollBarUpBtnNormalImageBorderZ,
		PropertyComboBoxScrollBarUpBtnNormalImageBorderW,
		PropertyComboBoxScrollBarUpBtnDisabledImagePath,
		PropertyComboBoxScrollBarUpBtnDisabledImageRect,
		PropertyComboBoxScrollBarUpBtnDisabledImageRectLeft,
		PropertyComboBoxScrollBarUpBtnDisabledImageRectTop,
		PropertyComboBoxScrollBarUpBtnDisabledImageRectWidth,
		PropertyComboBoxScrollBarUpBtnDisabledImageRectHeight,
		PropertyComboBoxScrollBarUpBtnDisabledImageBorder,
		PropertyComboBoxScrollBarUpBtnDisabledImageBorderX,
		PropertyComboBoxScrollBarUpBtnDisabledImageBorderY,
		PropertyComboBoxScrollBarUpBtnDisabledImageBorderZ,
		PropertyComboBoxScrollBarUpBtnDisabledImageBorderW,
		PropertyComboBoxScrollBarDownBtnNormalImagePath,
		PropertyComboBoxScrollBarDownBtnNormalImageRect,
		PropertyComboBoxScrollBarDownBtnNormalImageRectLeft,
		PropertyComboBoxScrollBarDownBtnNormalImageRectTop,
		PropertyComboBoxScrollBarDownBtnNormalImageRectWidth,
		PropertyComboBoxScrollBarDownBtnNormalImageRectHeight,
		PropertyComboBoxScrollBarDownBtnNormalImageBorder,
		PropertyComboBoxScrollBarDownBtnNormalImageBorderX,
		PropertyComboBoxScrollBarDownBtnNormalImageBorderY,
		PropertyComboBoxScrollBarDownBtnNormalImageBorderZ,
		PropertyComboBoxScrollBarDownBtnNormalImageBorderW,
		PropertyComboBoxScrollBarDownBtnDisabledImagePath,
		PropertyComboBoxScrollBarDownBtnDisabledImageRect,
		PropertyComboBoxScrollBarDownBtnDisabledImageRectLeft,
		PropertyComboBoxScrollBarDownBtnDisabledImageRectTop,
		PropertyComboBoxScrollBarDownBtnDisabledImageRectWidth,
		PropertyComboBoxScrollBarDownBtnDisabledImageRectHeight,
		PropertyComboBoxScrollBarDownBtnDisabledImageBorder,
		PropertyComboBoxScrollBarDownBtnDisabledImageBorderX,
		PropertyComboBoxScrollBarDownBtnDisabledImageBorderY,
		PropertyComboBoxScrollBarDownBtnDisabledImageBorderZ,
		PropertyComboBoxScrollBarDownBtnDisabledImageBorderW,
		PropertyComboBoxScrollBarThumbBtnNormalImagePath,
		PropertyComboBoxScrollBarThumbBtnNormalImageRect,
		PropertyComboBoxScrollBarThumbBtnNormalImageRectLeft,
		PropertyComboBoxScrollBarThumbBtnNormalImageRectTop,
		PropertyComboBoxScrollBarThumbBtnNormalImageRectWidth,
		PropertyComboBoxScrollBarThumbBtnNormalImageRectHeight,
		PropertyComboBoxScrollBarThumbBtnNormalImageBorder,
		PropertyComboBoxScrollBarThumbBtnNormalImageBorderX,
		PropertyComboBoxScrollBarThumbBtnNormalImageBorderY,
		PropertyComboBoxScrollBarThumbBtnNormalImageBorderZ,
		PropertyComboBoxScrollBarThumbBtnNormalImageBorderW,
		PropertyComboBoxScrollBarImagePath,
		PropertyComboBoxScrollBarImageRect,
		PropertyComboBoxScrollBarImageRectLeft,
		PropertyComboBoxScrollBarImageRectTop,
		PropertyComboBoxScrollBarImageRectWidth,
		PropertyComboBoxScrollBarImageRectHeight,
		PropertyComboBoxScrollBarImageBorder,
		PropertyComboBoxScrollBarImageBorderX,
		PropertyComboBoxScrollBarImageBorderY,
		PropertyComboBoxScrollBarImageBorderZ,
		PropertyComboBoxScrollBarImageBorderW,
		PropertyListBoxScrollbarWidth,
		PropertyListBoxScrollbarUpDownBtnHeight,
		PropertyListBoxItemSize,
		PropertyListBoxItemSizeX,
		PropertyListBoxItemSizeY,
		PropertyListBoxItemCount,
		PropertyListBoxScrollBarUpBtnNormalImagePath,
		PropertyListBoxScrollBarUpBtnNormalImageRect,
		PropertyListBoxScrollBarUpBtnNormalImageRectLeft,
		PropertyListBoxScrollBarUpBtnNormalImageRectTop,
		PropertyListBoxScrollBarUpBtnNormalImageRectWidth,
		PropertyListBoxScrollBarUpBtnNormalImageRectHeight,
		PropertyListBoxScrollBarUpBtnNormalImageBorder,
		PropertyListBoxScrollBarUpBtnNormalImageBorderX,
		PropertyListBoxScrollBarUpBtnNormalImageBorderY,
		PropertyListBoxScrollBarUpBtnNormalImageBorderZ,
		PropertyListBoxScrollBarUpBtnNormalImageBorderW,
		PropertyListBoxScrollBarUpBtnDisabledImagePath,
		PropertyListBoxScrollBarUpBtnDisabledImageRect,
		PropertyListBoxScrollBarUpBtnDisabledImageRectLeft,
		PropertyListBoxScrollBarUpBtnDisabledImageRectTop,
		PropertyListBoxScrollBarUpBtnDisabledImageRectWidth,
		PropertyListBoxScrollBarUpBtnDisabledImageRectHeight,
		PropertyListBoxScrollBarUpBtnDisabledImageBorder,
		PropertyListBoxScrollBarUpBtnDisabledImageBorderX,
		PropertyListBoxScrollBarUpBtnDisabledImageBorderY,
		PropertyListBoxScrollBarUpBtnDisabledImageBorderZ,
		PropertyListBoxScrollBarUpBtnDisabledImageBorderW,
		PropertyListBoxScrollBarDownBtnNormalImagePath,
		PropertyListBoxScrollBarDownBtnNormalImageRect,
		PropertyListBoxScrollBarDownBtnNormalImageRectLeft,
		PropertyListBoxScrollBarDownBtnNormalImageRectTop,
		PropertyListBoxScrollBarDownBtnNormalImageRectWidth,
		PropertyListBoxScrollBarDownBtnNormalImageRectHeight,
		PropertyListBoxScrollBarDownBtnNormalImageBorder,
		PropertyListBoxScrollBarDownBtnNormalImageBorderX,
		PropertyListBoxScrollBarDownBtnNormalImageBorderY,
		PropertyListBoxScrollBarDownBtnNormalImageBorderZ,
		PropertyListBoxScrollBarDownBtnNormalImageBorderW,
		PropertyListBoxScrollBarDownBtnDisabledImagePath,
		PropertyListBoxScrollBarDownBtnDisabledImageRect,
		PropertyListBoxScrollBarDownBtnDisabledImageRectLeft,
		PropertyListBoxScrollBarDownBtnDisabledImageRectTop,
		PropertyListBoxScrollBarDownBtnDisabledImageRectWidth,
		PropertyListBoxScrollBarDownBtnDisabledImageRectHeight,
		PropertyListBoxScrollBarDownBtnDisabledImageBorder,
		PropertyListBoxScrollBarDownBtnDisabledImageBorderX,
		PropertyListBoxScrollBarDownBtnDisabledImageBorderY,
		PropertyListBoxScrollBarDownBtnDisabledImageBorderZ,
		PropertyListBoxScrollBarDownBtnDisabledImageBorderW,
		PropertyListBoxScrollBarThumbBtnNormalImagePath,
		PropertyListBoxScrollBarThumbBtnNormalImageRect,
		PropertyListBoxScrollBarThumbBtnNormalImageRectLeft,
		PropertyListBoxScrollBarThumbBtnNormalImageRectTop,
		PropertyListBoxScrollBarThumbBtnNormalImageRectWidth,
		PropertyListBoxScrollBarThumbBtnNormalImageRectHeight,
		PropertyListBoxScrollBarThumbBtnNormalImageBorder,
		PropertyListBoxScrollBarThumbBtnNormalImageBorderX,
		PropertyListBoxScrollBarThumbBtnNormalImageBorderY,
		PropertyListBoxScrollBarThumbBtnNormalImageBorderZ,
		PropertyListBoxScrollBarThumbBtnNormalImageBorderW,
		PropertyListBoxScrollBarImagePath,
		PropertyListBoxScrollBarImageRect,
		PropertyListBoxScrollBarImageRectLeft,
		PropertyListBoxScrollBarImageRectTop,
		PropertyListBoxScrollBarImageRectWidth,
		PropertyListBoxScrollBarImageRectHeight,
		PropertyListBoxScrollBarImageBorder,
		PropertyListBoxScrollBarImageBorderX,
		PropertyListBoxScrollBarImageBorderY,
		PropertyListBoxScrollBarImageBorderZ,
		PropertyListBoxScrollBarImageBorderW,
		PropertyDialogEnableDrag,
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
	void UpdatePropertiesCloth(CMFCPropertyGridProperty * pComponent, ClothComponent * cloth_cmp);
	void UpdatePropertiesStaticEmitter(CMFCPropertyGridProperty * pComponent, StaticEmitter * emit_cmp);
	void UpdatePropertiesStaticEmitterParticle(CMFCPropertyGridProperty * pParticle, const CPoint & chunkid, int instid, my::Emitter::Particle * particle);
	void UpdatePropertiesSphericalEmitter(CMFCPropertyGridProperty * pComponent, SphericalEmitter * sphe_emit_cmp);
	void UpdatePropertiesSpline(CMFCPropertyGridProperty * pSpline, my::Spline * spline);
	void UpdatePropertiesSplineNode(CMFCPropertyGridProperty * pSpline, int NodeId, const my::SplineNode * node);
	void UpdatePropertiesTerrain(CMFCPropertyGridProperty * pComponent, Terrain * terrain);
	void UpdatePropertiesAnimator(CMFCPropertyGridProperty * pComponent, Animator * animator);
	void UpdatePropertiesAnimationNode(CMFCPropertyGridProperty * pAnimationNode, AnimationNode * node);
	void UpdatePropertiesAnimationNodeSequence(CMFCPropertyGridProperty * pAnimationNode, AnimationNodeSequence * seq);
	void UpdatePropertiesControl(my::Control * control);
	void UpdatePropertiesStatic(CMFCPropertyGridProperty * pControl, my::Static * static_ctl);
	void UpdatePropertiesProgressBar(CMFCPropertyGridProperty * pControl, my::ProgressBar * progressbar);
	void UpdatePropertiesButton(CMFCPropertyGridProperty * pControl, my::Button * button);
	void UpdatePropertiesEditBox(CMFCPropertyGridProperty * pControl, my::EditBox * editbox);
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
	void CreatePropertiesCloth(CMFCPropertyGridProperty * pComponent, ClothComponent * cloth_cmp);
	void CreatePropertiesStaticEmitter(CMFCPropertyGridProperty * pComponent, StaticEmitter * emit_cmp);
	void CreatePropertiesStaticEmitterParticle(CMFCPropertyGridProperty * pParentProp, const CPoint & chunkid, int instid, my::Emitter::Particle * particle);
	void CreatePropertiesSphericalEmitter(CMFCPropertyGridProperty * pComponent, SphericalEmitter * sphe_emit_cmp);
	void CreatePropertiesSpline(CMFCPropertyGridProperty * pParentProp, LPCTSTR lpszName, Property PropertyId, my::Spline * spline);
	void CreatePropertiesSplineNode(CMFCPropertyGridProperty * pSpline, int NodeId, my::SplineNode * node);
	void CreatePropertiesTerrain(CMFCPropertyGridProperty * pComponent, Terrain * terrain);
	void CreatePropertiesAnimator(CMFCPropertyGridProperty * pComponent, Animator * animator);
	void CreatePropertiesAnimationNode(CMFCPropertyGridProperty * pParentCtrl, AnimationNode * node);
	void CreatePropertiesAnimationNodeSequence(CMFCPropertyGridProperty * pAnimationNode, AnimationNodeSequence * seq);
	void CreatePropertiesControl(my::Control * control);
	void CreatePropertiesStatic(CMFCPropertyGridProperty * pControl, my::Static * static_ctl);
	void CreatePropertiesProgressBar(CMFCPropertyGridProperty * pControl, my::ProgressBar * progressbar);
	void CreatePropertiesButton(CMFCPropertyGridProperty * pControl, my::Button * button);
	void CreatePropertiesEditBox(CMFCPropertyGridProperty * pControl, my::EditBox * editbox);
	void CreatePropertiesCheckBox(CMFCPropertyGridProperty * pControl, my::CheckBox * checkbox);
	void CreatePropertiesComboBox(CMFCPropertyGridProperty * pControl, my::ComboBox * combobox);
	void CreatePropertiesListBox(CMFCPropertyGridProperty * pControl, my::ListBox * listbox);
	void CreatePropertiesDialog(CMFCPropertyGridProperty * pControl, my::Dialog * dialog);

	static Property GetComponentProp(DWORD type);
	static unsigned int GetComponentPropCount(DWORD type);
	static LPCTSTR GetComponentTypeName(DWORD type);
	static TerrainChunk * GetTerrainChunkSafe(Terrain * terrain, const CPoint & chunkid);
	static Property GetMaterialParameterTypeProp(DWORD type);
	static unsigned int GetControlPropCount(DWORD type);
	static LPCTSTR GetControlTypeName(DWORD type);

	void UpdatePropertiesPaintTool(void);
	void CreatePropertiesPaintTool(void);

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

