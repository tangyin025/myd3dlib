#include "Terrain2.h"

void Terrain2::RequestResource(void)
{
	Component::RequestResource();
}

void Terrain2::ReleaseResource(void)
{
	Component::ReleaseResource();
}

void Terrain2::OnSetShader(IDirect3DDevice9 * pd3dDevice, my::Effect * shader, LPARAM lparam)
{

}

void Terrain2::OnShaderChanged(void)
{

}

void Terrain2::Update(float fElapsedTime)
{

}

void Terrain2::AddToPipeline(const my::Frustum & frustum, RenderPipeline * pipeline, unsigned int PassMask, const my::Vector3 & ViewPos, const my::Vector3 & TargetPos)
{

}

void Terrain2::ClearShape(void)
{
	Component::ClearShape();
}
