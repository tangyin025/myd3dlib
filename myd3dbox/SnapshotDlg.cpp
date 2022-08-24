// CSnapshotDlg.cpp : implementation file
//

#include "stdafx.h"
#include "SnapshotDlg.h"
#include "afxdialogex.h"
#include "MainApp.h"
#include "MainFrm.h"
#include "Actor.h"


// CSnapshotDlg dialog

IMPLEMENT_DYNAMIC(CSnapshotDlg, CDialogEx)

CSnapshotDlg::CSnapshotDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_DIALOG7, pParent)
{

}

CSnapshotDlg::~CSnapshotDlg()
{
}

void CSnapshotDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CSnapshotDlg, CDialogEx)
END_MESSAGE_MAP()


// CSnapshotDlg message handlers


BOOL CSnapshotDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// TODO:  Add extra initialization here

	return TRUE;  // return TRUE unless you set the focus to a control
				  // EXCEPTION: OCX Property Pages should return FALSE
}


void CSnapshotDlg::OnOK()
{
	// TODO: Add your specialized code here and/or call the base class
	my::Texture2D rt;
	rt.CreateTexture(1024, 1024, 1, D3DUSAGE_RENDERTARGET, D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT);
	const D3DSURFACE_DESC desc = rt.GetLevelDesc();
	CComPtr<IDirect3DSurface9> rtsurf = rt.GetSurfaceLevel(0);

	my::Surface DepthStencil;
	DepthStencil.CreateDepthStencilSurface(
		desc.Width, desc.Height, D3DFMT_D24X8, D3DMULTISAMPLE_NONE, 0);

	struct RenderContext : RenderPipeline::IRenderContext
	{
		virtual void QueryRenderComponent(const my::Frustum& frustum, RenderPipeline* pipeline, unsigned int PassMask)
		{
			struct Callback : public my::OctNode::QueryCallback
			{
				const my::Frustum& frustum;
				RenderPipeline* pipeline;
				unsigned int PassMask;
				const my::Vector3& ViewPos;
				const my::Vector3& TargetPos;
				Callback(const my::Frustum& _frustum, RenderPipeline* _pipeline, unsigned int _PassMask, const my::Vector3& _ViewPos, const my::Vector3& _TargetPos)
					: frustum(_frustum)
					, pipeline(_pipeline)
					, PassMask(_PassMask)
					, ViewPos(_ViewPos)
					, TargetPos(_TargetPos)
				{
				}

				virtual bool OnQueryEntity(my::OctEntity* oct_entity, const my::AABB& aabb, my::IntersectionTests::IntersectionType)
				{
					ASSERT(dynamic_cast<Actor*>(oct_entity));

					Actor* actor = static_cast<Actor*>(oct_entity);

					if (actor->IsRequested())
					{
						actor->AddToPipeline(frustum, pipeline, PassMask, ViewPos, TargetPos);
					}
					return true;
				}
			};

			CMainFrame* pFrame = DYNAMIC_DOWNCAST(CMainFrame, AfxGetMainWnd());
			ASSERT_VALID(pFrame);
			Callback cb(frustum, pipeline, PassMask, m_Camera->m_Eye, m_Camera->m_Eye);
			pFrame->QueryEntity(frustum, &cb);
		}
	};

	RenderContext rc;
	rc.m_Camera.reset(new my::OrthoCamera(30, 30, -1000, 3000));
	rc.m_Camera->UpdateViewProj();
	rc.m_NormalRT.reset(new my::Texture2D());
	rc.m_NormalRT->CreateTexture(
		desc.Width, desc.Height, 1, D3DUSAGE_RENDERTARGET, D3DFMT_A32B32G32R32F, D3DPOOL_DEFAULT);
	rc.m_PositionRT.reset(new my::Texture2D());
	rc.m_PositionRT->CreateTexture(
		desc.Width, desc.Height, 1, D3DUSAGE_RENDERTARGET, D3DFMT_A32B32G32R32F, D3DPOOL_DEFAULT);
	rc.m_LightRT.reset(new my::Texture2D());
	rc.m_LightRT->CreateTexture(
		desc.Width, desc.Height, 1, D3DUSAGE_RENDERTARGET, D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT);
	for (unsigned int i = 0; i < RenderPipeline::RTChain::RTArray::static_size; i++)
	{
		rc.m_OpaqueRT.m_RenderTarget[i].reset(new my::Texture2D());
		rc.m_OpaqueRT.m_RenderTarget[i]->CreateTexture(
			desc.Width, desc.Height, 1, D3DUSAGE_RENDERTARGET, D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT);
	}

	D3DVIEWPORT9 vp = { 0, 0, 1024, 1024, 0.0, 1.0f };
	theApp.OnRender(theApp.m_d3dDevice, rtsurf, DepthStencil.m_ptr, &vp, &rc, 0, 0);
	HRESULT hr;
	V(D3DXSaveTextureToFileA("aaa.bmp", D3DXIFF_BMP, rt.m_ptr, NULL));

	CDialogEx::OnOK();
}
