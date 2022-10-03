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
	, m_TexWidth(1024)
	, m_TexHeight(1024)
	, m_SnapPos(0, 0)
	, m_SnapSize(8192, 8192)
	, m_SnapCol(1)
	, m_SnapRow(1)
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
	DDX_Text(pDX, IDC_EDIT4, m_SnapPos.x);
	DDX_Text(pDX, IDC_EDIT5, m_SnapSize.x);
	DDV_MinMaxFloat(pDX, m_SnapSize.x, EPSILON_E6, FLT_MAX);
	DDX_Text(pDX, IDC_EDIT6, m_SnapCol);
	DDV_MinMaxInt(pDX, m_SnapCol, 1, INT_MAX);
	DDX_Text(pDX, IDC_EDIT7, m_SnapPos.y);
	DDX_Text(pDX, IDC_EDIT8, m_SnapSize.y);
	DDV_MinMaxFloat(pDX, m_SnapSize.y, EPSILON_E6, FLT_MAX);
	DDX_Text(pDX, IDC_EDIT9, m_SnapRow);
	DDV_MinMaxInt(pDX, m_SnapRow, 1, INT_MAX);

	if (pDX->m_bSaveAndValidate)
	{
		theApp.WriteProfileString(_T("Settings"), _T("SnapshotPath"), m_TexPath);
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
	rc.m_Camera.reset(new my::OrthoCamera(m_SnapSize.x, m_SnapSize.y, -2000, 2000));
	rc.m_Camera->m_Eye = my::Vector3(m_SnapPos, 0);
	rc.m_Camera->m_Euler = my::Vector3(D3DXToRadian(-90), 0, 0);
	rc.m_Camera->UpdateViewProj();
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

	D3DVIEWPORT9 vp = { 0, 0, desc.Width, desc.Height, 0.0, 1.0f };
	theApp.OnRender(theApp.m_d3dDevice, rtsurf, DepthStencil.m_ptr, &vp, &rc, 0, 0);
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
