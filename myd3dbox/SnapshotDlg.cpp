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
	, m_TexPath(theApp.GetProfileString(_T("Settings"), _T("SnapshotPath"), _T("aaa.png")))
	, m_TexWidth(theApp.GetProfileInt(_T("Settings"), _T("SnapshotWidth"), 1024))
	, m_TexHeight(theApp.GetProfileInt(_T("Settings"), _T("SnapshotHeight"), 1024))
	, m_SnapArea(-4096, -4096, 4096, 4096)
{

}

CSnapshotDlg::~CSnapshotDlg()
{
}

void CSnapshotDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT1, m_TexPath);
	DDX_Text(pDX, IDC_EDIT2, m_TexWidth);
	DDV_MinMaxInt(pDX, m_TexWidth, 1, INT_MAX);
	DDX_Text(pDX, IDC_EDIT3, m_TexHeight);
	DDV_MinMaxInt(pDX, m_TexHeight, 1, INT_MAX);
	DDX_Text(pDX, IDC_EDIT4, m_SnapArea.l);
	DDX_Text(pDX, IDC_EDIT5, m_SnapArea.t);
	DDX_Text(pDX, IDC_EDIT6, m_SnapArea.r);
	DDX_Text(pDX, IDC_EDIT7, m_SnapArea.b);

	if (pDX->m_bSaveAndValidate)
	{
		theApp.WriteProfileString(_T("Settings"), _T("SnapshotPath"), m_TexPath);
		theApp.WriteProfileInt(_T("Settings"), _T("SnapshotWidth"), m_TexWidth);
		theApp.WriteProfileInt(_T("Settings"), _T("SnapshotHeight"), m_TexHeight);
	}
}


BEGIN_MESSAGE_MAP(CSnapshotDlg, CDialogEx)
	ON_BN_CLICKED(IDC_BUTTON1, &CSnapshotDlg::OnClickedButton1)
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
	if (!UpdateData(TRUE))
	{
		TRACE(traceAppMsg, 0, "UpdateData failed during dialog termination.\n");
		// the UpdateData routine will set focus to correct item
		return;
	}

	my::Texture2D rt;
	rt.CreateTexture(m_TexWidth, m_TexHeight, 1, D3DUSAGE_RENDERTARGET, D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT);
	CComPtr<IDirect3DSurface9> rtsurf = rt.GetSurfaceLevel(0);
	D3DSURFACE_DESC desc;
	HRESULT hr;
	V(rtsurf->GetDesc(&desc));

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
	rc.m_Camera.reset(new my::OrthoCamera(m_SnapArea.Width(), m_SnapArea.Height(), -2000, 2000));
	my::OrthoCamera* ortho_camera = dynamic_cast<my::OrthoCamera*>(rc.m_Camera.get());
	ortho_camera->m_Eye = my::Vector3(0, 0, 0);
	ortho_camera->m_Euler = my::Vector3(D3DXToRadian(-90), 0, 0);
	const my::Matrix4 Rotation = my::Matrix4::RotationYawPitchRoll(ortho_camera->m_Euler.y, ortho_camera->m_Euler.x, ortho_camera->m_Euler.z);
	ortho_camera->m_View = (Rotation * my::Matrix4::Translation(ortho_camera->m_Eye)).inverse();
	const my::Rectangle Rect = my::Rectangle::LeftTop(m_SnapArea.l, m_SnapArea.t, ortho_camera->m_Width, ortho_camera->m_Height);
	ortho_camera->m_Proj = my::Matrix4::OrthoOffCenterRH(Rect.l, Rect.r, Rect.t, Rect.b, ortho_camera->m_Nz, ortho_camera->m_Fz);
	ortho_camera->m_ViewProj = ortho_camera->m_View * ortho_camera->m_Proj;
	ortho_camera->m_InverseViewProj = ortho_camera->m_ViewProj.inverse();
	rc.m_NormalRT.reset(new my::Texture2D());
	rc.m_NormalRT->CreateTexture(
		desc.Width, desc.Height, 1, D3DUSAGE_RENDERTARGET, D3DFMT_A32B32G32R32F, D3DPOOL_DEFAULT);
	rc.m_SpecularRT.reset(new my::Texture2D());
	rc.m_SpecularRT->CreateTexture(
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

	theApp.OnRender(theApp.m_d3dDevice, rtsurf, DepthStencil.m_ptr, &desc, &rc, 0, 0);
	CString ext(PathFindExtension(m_TexPath));
	V(D3DXSaveTextureToFile(m_TexPath, ext.CompareNoCase(_T(".png")) == 0 ? D3DXIFF_PNG : D3DXIFF_BMP, rt.m_ptr, NULL));

	CDialogEx::OnOK();
}


void CSnapshotDlg::OnClickedButton1()
{
	// TODO: Add your control notification handler code here
	CFileDialog dlg(FALSE, NULL, m_TexPath, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, NULL, this);
	if (dlg.DoModal() == IDOK)
	{
		SetDlgItemText(IDC_EDIT1, dlg.GetPathName());
	}
}
