#pragma once

class RenderPipeline
{
public:
	enum MeshType
	{
		MeshTypeStatic,
		MeshTypeAnimation,
	};

	enum DrawStage
	{
		DrawStageShadow,
		DrawStageNBuffer,
		DrawStageDBuffer,
		DrawStageCBuffer,
	};

	class IShaderSetter
	{
	public:
		virtual void OnSetShader(my::Effect * shader, DWORD AttribId) = 0;
	};

	typedef boost::tuple<my::Mesh *, DWORD, my::Effect *, IShaderSetter *> OpaqueRenderAtom;

	typedef std::vector<OpaqueRenderAtom> OpaqueRenderAtomList;

	OpaqueRenderAtomList m_OpaqueList;

public:
	virtual my::Effect * QueryShader(MeshType mesh_type, DrawStage draw_stage, const my::Material * material) = 0;

	void OnRender(IDirect3DDevice9 * pd3dDevice, double fTime, float fElapsedTime);

	void RenderOpaqueMesh(my::Mesh * mesh, DWORD AttribId, my::Effect * shader, IShaderSetter * setter);

	void PushOpaqueMesh(my::Mesh * mesh, DWORD AttribId, my::Effect * shader, IShaderSetter * setter);

	void ClearAllOpaqueMeshes(void);
};
