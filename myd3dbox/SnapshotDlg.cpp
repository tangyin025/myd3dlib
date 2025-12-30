// Copyright (c) 2011-2024 tangyin025
// License: MIT

#include "stdafx.h"
#include "SnapshotDlg.h"
#include "afxdialogex.h"
#include "MainApp.h"
#include "MainFrm.h"
#include "Actor.h"
#include "NavigationSerialization.h"
#include "ChildView.h"


// CSnapshotDlg dialog

IMPLEMENT_DYNAMIC(CSnapshotDlg, CDialogEx)

CSnapshotDlg::CSnapshotDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_DIALOG7, pParent)
	, m_TexPath(theApp.GetProfileString(_T("Settings"), _T("SnapshotPath"), _T("aaa.png")))
	, m_TexWidth(theApp.GetProfileInt(_T("Settings"), _T("SnapshotWidth"), 1024))
	, m_TexHeight(theApp.GetProfileInt(_T("Settings"), _T("SnapshotHeight"), 1024))
	, m_duDebugDrawPrimitives(DU_DRAW_QUADS + 1)
	, m_SnapArea(-4096 + 4, 4096 - 4, 4096 + 4, -4096 - 4)
	, m_SnapEye(0, 0, 0)
	, m_SnapEular(D3DXToRadian(-90), 0, 0)
	, m_RTType(theApp.GetProfileInt(_T("Settings"), _T("SnapshotRTTypt"), RenderPipeline::RenderTargetOpaque))
	, m_OpenImage(theApp.GetProfileInt(_T("Settings"), _T("SnapshotOpenImage"), TRUE))
	, m_UseOrthoCamera(theApp.GetProfileInt(_T("Settings"), _T("SnapshotUseOrthoCamera"), TRUE))
{
	BYTE* pData;
	UINT n;
	if (theApp.GetProfileBinary(_T("Settings"), _T("SnapshotArea"), &pData, &n))
	{
		ASSERT(n == sizeof(m_SnapArea));
		m_SnapArea = *(my::Rectangle*)pData;
		delete[] pData; // free the buffer
	}
	if (theApp.GetProfileBinary(_T("Settings"), _T("SnapshotComponentTypes"), &pData, &n))
	{
		ASSERT(n == sizeof(m_ComponentTypes));
		std::copy((BOOL*)pData, (BOOL*)pData + _countof(m_ComponentTypes), &m_ComponentTypes[0]);
		delete[] pData;
	}
	else
	{
		std::fill_n(m_ComponentTypes, _countof(m_ComponentTypes), TRUE);
	}
}

CSnapshotDlg::~CSnapshotDlg()
{
}

void AFXAPI _AfxTextDegreeFormat(CDataExchange* pDX, int nIDC,
	void* pData, double value, int nSizeGcvt)
{
	ASSERT(pData != NULL);

	pDX->PrepareEditCtrl(nIDC);
	HWND hWndCtrl;
	pDX->m_pDlgWnd->GetDlgItem(nIDC, &hWndCtrl);

	const int TEXT_BUFFER_SIZE = 400;
	TCHAR szBuffer[TEXT_BUFFER_SIZE];
	if (pDX->m_bSaveAndValidate)
	{
		::GetWindowText(hWndCtrl, szBuffer, _countof(szBuffer));
		double d;
		if (_sntscanf_s(szBuffer, _countof(szBuffer), _T("%lf"), &d) != 1)
		{
			AfxMessageBox(AFX_IDP_PARSE_REAL);
			pDX->Fail();            // throws exception
		}
		if (nSizeGcvt == FLT_DIG)
			*((float*)pData) = D3DXToRadian((float)d);
		else
			*((double*)pData) = D3DXToRadian(d);
	}
	else
	{
		ATL_CRT_ERRORCHECK_SPRINTF(_sntprintf_s(szBuffer, _countof(szBuffer), _countof(szBuffer) - 1, _T("%.*g"), nSizeGcvt, D3DXToDegree(value)));
		AfxSetWindowText(hWndCtrl, szBuffer);
	}
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
	DDX_Text(pDX, IDC_EDIT8, m_SnapEye.x);
	DDX_Text(pDX, IDC_EDIT9, m_SnapEye.y);
	DDX_Text(pDX, IDC_EDIT10, m_SnapEye.z);
	_AfxTextDegreeFormat(pDX, IDC_EDIT11, &m_SnapEular.x, m_SnapEular.x, FLT_DIG);
	_AfxTextDegreeFormat(pDX, IDC_EDIT12, &m_SnapEular.y, m_SnapEular.y, FLT_DIG);
	_AfxTextDegreeFormat(pDX, IDC_EDIT13, &m_SnapEular.z, m_SnapEular.z, FLT_DIG);
	DDX_Check(pDX, IDC_CHECK1, m_ComponentTypes[0]);
	DDX_Check(pDX, IDC_CHECK2, m_ComponentTypes[1]);
	DDX_Check(pDX, IDC_CHECK3, m_ComponentTypes[2]);
	DDX_Check(pDX, IDC_CHECK4, m_ComponentTypes[3]);
	DDX_Check(pDX, IDC_CHECK5, m_ComponentTypes[4]);
	DDX_Check(pDX, IDC_CHECK6, m_ComponentTypes[5]);
	DDX_Check(pDX, IDC_CHECK7, m_ComponentTypes[6]);
	DDX_Check(pDX, IDC_CHECK8, m_ComponentTypes[7]);
	DDX_Check(pDX, IDC_CHECK9, m_ComponentTypes[8]);
	DDX_Radio(pDX, IDC_RADIO1, m_RTType);
	DDX_Check(pDX, IDC_CHECK10, m_OpenImage);
	DDX_Check(pDX, IDC_CHECK11, m_UseOrthoCamera);

	if (pDX->m_bSaveAndValidate)
	{
		theApp.WriteProfileString(_T("Settings"), _T("SnapshotPath"), m_TexPath);
		theApp.WriteProfileInt(_T("Settings"), _T("SnapshotWidth"), m_TexWidth);
		theApp.WriteProfileInt(_T("Settings"), _T("SnapshotHeight"), m_TexHeight);
		theApp.WriteProfileBinary(_T("Settings"), _T("SnapshotArea"), (LPBYTE)&m_SnapArea, sizeof(m_SnapArea));
		theApp.WriteProfileBinary(_T("Settings"), _T("SnapshotComponentTypes"), (LPBYTE)&m_ComponentTypes, sizeof(m_ComponentTypes));
		theApp.WriteProfileInt(_T("Settings"), _T("SnapshotRTTypt"), m_RTType);
		theApp.WriteProfileInt(_T("Settings"), _T("SnapshotOpenImage"), m_OpenImage);
		theApp.WriteProfileInt(_T("Settings"), _T("SnapshotUseOrthoCamera"), m_UseOrthoCamera);
	}
}

#define DUCOLOR_TO_D3DCOLOR(col) ((col & 0xff00ff00) | (col & 0x00ff0000) >> 16 | (col & 0x000000ff) << 16)

void CSnapshotDlg::depthMask(bool state)
{
}

void CSnapshotDlg::texture(bool state)
{
}

void CSnapshotDlg::begin(duDebugDrawPrimitives prim, float size)
{
	m_duDebugDrawPrimitives = prim;
}

void CSnapshotDlg::vertex(const float* pos, unsigned int color)
{
	if (m_duDebugDrawPrimitives == DU_DRAW_LINES)
	{
		PushLineVertex(pos[0], pos[1], pos[2], DUCOLOR_TO_D3DCOLOR(color));
	}
	else if (m_duDebugDrawPrimitives == DU_DRAW_TRIS)
	{
		PushTriangleVertex(pos[0], pos[1], pos[2], DUCOLOR_TO_D3DCOLOR(color));
	}
}

void CSnapshotDlg::vertex(const float x, const float y, const float z, unsigned int color)
{
	if (m_duDebugDrawPrimitives == DU_DRAW_LINES)
	{
		PushLineVertex(x, y, z, DUCOLOR_TO_D3DCOLOR(color));
	}
	else if (m_duDebugDrawPrimitives == DU_DRAW_TRIS)
	{
		PushTriangleVertex(x, y, z, DUCOLOR_TO_D3DCOLOR(color));
	}
}

void CSnapshotDlg::vertex(const float* pos, unsigned int color, const float* uv)
{
	if (m_duDebugDrawPrimitives == DU_DRAW_LINES)
	{
		PushLineVertex(pos[0], pos[1], pos[2], DUCOLOR_TO_D3DCOLOR(color));
	}
	else if (m_duDebugDrawPrimitives == DU_DRAW_TRIS)
	{
		PushTriangleVertex(pos[0], pos[1], pos[2], DUCOLOR_TO_D3DCOLOR(color));
	}
}

void CSnapshotDlg::vertex(const float x, const float y, const float z, unsigned int color, const float u, const float v)
{
	if (m_duDebugDrawPrimitives == DU_DRAW_LINES)
	{
		PushLineVertex(x, y, z, DUCOLOR_TO_D3DCOLOR(color));
	}
	else if (m_duDebugDrawPrimitives == DU_DRAW_TRIS)
	{
		PushTriangleVertex(x, y, z, DUCOLOR_TO_D3DCOLOR(color));
	}
}

void CSnapshotDlg::end()
{
	m_duDebugDrawPrimitives = DU_DRAW_QUADS + 1;
}

unsigned int CSnapshotDlg::areaToCol(unsigned int area)
{
	return duDebugDraw::areaToCol(area);
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

	DoSnapshot();

	if (m_OpenImage)
	{
		::ShellExecute(AfxGetMainWnd()->m_hWnd, _T("open"), m_TexPath, NULL, NULL, SW_SHOWNORMAL);
	}

	//CDialogEx::OnOK();
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


void CSnapshotDlg::DoSnapshot()
{
	// TODO: Add your implementation code here.
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
		CSnapshotDlg* pDlg;

		RenderContext(CSnapshotDlg* _pDlg)
			: pDlg(_pDlg)
		{
		}

		virtual void QueryRenderComponent(const my::Frustum& frustum, RenderPipeline* pipeline, unsigned int PassMask)
		{
			struct Callback : public my::OctNode::QueryCallback
			{
				const my::Frustum& frustum;
				RenderPipeline* pipeline;
				unsigned int PassMask;
				CMainFrame* pFrame;
				CChildView* pView;
				CSnapshotDlg* pDlg;
				RenderContext* rc;

				Callback(const my::Frustum& _frustum, RenderPipeline* _pipeline, unsigned int _PassMask, CMainFrame* _pFrame, CChildView* _pView, CSnapshotDlg* _pDlg, RenderContext* _rc)
					: frustum(_frustum)
					, pipeline(_pipeline)
					, PassMask(_PassMask)
					, pFrame(_pFrame)
					, pView(_pView)
					, pDlg(_pDlg)
					, rc(_rc)
				{
				}

				virtual bool OnQueryEntity(my::OctEntity* oct_entity, const my::AABB& aabb)
				{
					ASSERT(dynamic_cast<Actor*>(oct_entity));

					Actor* actor = static_cast<Actor*>(oct_entity);

					if (!actor->IsRequested())
					{
						_ASSERT(!actor->is_linked());

						actor->RequestResource();

						pFrame->m_ViewedActors.push_back(*actor);
					}

					const int Lod = pDlg->m_UseOrthoCamera ? 0 :
						my::Min(actor->CalculateLod((actor->m_OctAabb->Center() - rc->m_Camera->m_Eye).magnitude()), Actor::MaxLod - 1);

					my::ModelViewerCamera* model_view_camera = dynamic_cast<my::ModelViewerCamera*>(pView->m_Camera.get());

					Actor::ComponentPtrList::iterator cmp_iter = actor->m_Cmps.begin();
					for (; cmp_iter != actor->m_Cmps.end(); cmp_iter++)
					{
						if ((*cmp_iter)->m_LodMask & (1 << Lod)
							&& (*cmp_iter)->GetComponentType() >= Component::ComponentTypeMesh
							&& (*cmp_iter)->GetComponentType() <= Component::ComponentTypeNavigation
							&& pDlg->m_ComponentTypes[(*cmp_iter)->GetComponentType() - Component::ComponentTypeMesh])
						{
							if (!(*cmp_iter)->IsRequested())
							{
								(*cmp_iter)->RequestResource();

								theApp.m_d3dDeviceSec.Leave();
								theApp.CheckIORequests(0xFFFFFFFF); // ! INFINITE conflict with corecrt_math.h
								theApp.m_d3dDeviceSec.Enter();
							}

							if ((*cmp_iter)->GetComponentType() == Component::ComponentTypeNavigation
								&& PassMask & RenderPipeline::PassTypeToMask(RenderPipeline::PassTypeNormal))
							{
								Navigation* navi = dynamic_cast<Navigation*>(cmp_iter->get());
								navi->DebugDraw(pDlg, frustum, rc->m_Camera->m_Eye, model_view_camera->m_LookAt);
							}
							else
							{
								(*cmp_iter)->AddToPipeline(frustum, pipeline, PassMask, rc->m_Camera->m_Eye, model_view_camera->m_LookAt);
							}
						}
					}
					return true;
				}
			};

			CMainFrame* pFrame = DYNAMIC_DOWNCAST(CMainFrame, AfxGetMainWnd());
			ASSERT_VALID(pFrame);
			CChildView* pView = DYNAMIC_DOWNCAST(CChildView, pFrame->GetActiveView());
			ASSERT_VALID(pView);
			Callback cb(frustum, pipeline, PassMask, pFrame, pView, pDlg, this);
			pFrame->QueryEntity(frustum, &cb);
		}
	};

	CMainFrame* pFrame = DYNAMIC_DOWNCAST(CMainFrame, AfxGetMainWnd());
	ASSERT_VALID(pFrame);
	CChildView* pView = DYNAMIC_DOWNCAST(CChildView, pFrame->GetActiveView());
	ASSERT_VALID(pView);
	RenderContext rc(this);
	if (m_UseOrthoCamera)
	{
		rc.m_Camera.reset(new my::OrthoCamera(m_SnapArea.Width(), -m_SnapArea.Height(), -2000, 2000));
		my::OrthoCamera* ortho_camera = dynamic_cast<my::OrthoCamera*>(rc.m_Camera.get());
		ortho_camera->m_Eye = m_SnapEye;
		ortho_camera->m_Euler = m_SnapEular;
		const my::Matrix4 Rotation = my::Matrix4::RotationYawPitchRoll(ortho_camera->m_Euler.y, ortho_camera->m_Euler.x, ortho_camera->m_Euler.z);
		ortho_camera->m_View = (Rotation * my::Matrix4::Translation(ortho_camera->m_Eye)).inverse();
		ortho_camera->m_Proj = my::Matrix4::OrthoOffCenterRH(m_SnapArea.l, m_SnapArea.r, m_SnapArea.b, m_SnapArea.t, ortho_camera->m_Nz, ortho_camera->m_Fz);
		ortho_camera->m_ViewProj = ortho_camera->m_View * ortho_camera->m_Proj;
		ortho_camera->m_InverseViewProj = ortho_camera->m_ViewProj.inverse();
	}
	else
	{
		my::ModelViewerCamera* model_view_camera = dynamic_cast<my::ModelViewerCamera*>(pView->m_Camera.get());
		rc.m_Camera.reset(new my::PerspectiveCamera(model_view_camera->m_Fov, desc.Width / (float)desc.Height, model_view_camera->m_Nz, model_view_camera->m_Fz));
		my::PerspectiveCamera* persp_camera = dynamic_cast<my::PerspectiveCamera*>(rc.m_Camera.get());
		rc.m_Camera->m_Eye = m_SnapEye;
		rc.m_Camera->m_Euler = m_SnapEular;
		rc.m_Camera->m_View = my::Matrix4::Compose(my::Vector3::one, rc.m_Camera->m_Euler, rc.m_Camera->m_Eye).inverse();
		rc.m_Camera->m_Proj = my::Matrix4::PerspectiveFovRH(persp_camera->m_Fov, persp_camera->m_Aspect, rc.m_Camera->m_Nz, rc.m_Camera->m_Fz);
		rc.m_Camera->m_ViewProj = rc.m_Camera->m_View * rc.m_Camera->m_Proj;
		rc.m_Camera->m_InverseViewProj = rc.m_Camera->m_ViewProj.inverse();
	}
	rc.m_WireFrame = pView->m_WireFrame;
	rc.m_BloomEnable = pView->m_BloomEnable;
	rc.m_FxaaEnable = pView->m_FxaaEnable;
	rc.m_SsaoEnable = pView->m_SsaoEnable;
	rc.m_NormalRT.reset(new my::Texture2D());
	rc.m_NormalRT->CreateTexture(
		desc.Width, desc.Height, 1, D3DUSAGE_RENDERTARGET, D3DFMT_A32B32G32R32F, D3DPOOL_DEFAULT);
	rc.m_SpecularRT.reset(new my::Texture2D());
	rc.m_SpecularRT->CreateTexture(
		desc.Width, desc.Height, 1, D3DUSAGE_RENDERTARGET, D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT);
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
	rc.m_RTType = (RenderPipeline::RenderTargetType)m_RTType;

	CWaitCursor wait;
	theApp.OnRender(theApp.m_d3dDevice, rtsurf, DepthStencil.m_ptr, &desc, &rc, 0, 0);

	V(theApp.m_d3dDevice->SetVertexShader(NULL));
	V(theApp.m_d3dDevice->SetPixelShader(NULL));
	V(theApp.m_d3dDevice->SetTexture(0, NULL));
	V(theApp.m_d3dDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE));
	V(theApp.m_d3dDevice->SetRenderState(D3DRS_BLENDOP, D3DBLENDOP_ADD));
	V(theApp.m_d3dDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA));
	V(theApp.m_d3dDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA));
	V(theApp.m_d3dDevice->SetRenderState(D3DRS_ZENABLE, TRUE));
	V(theApp.m_d3dDevice->SetRenderState(D3DRS_ZWRITEENABLE, FALSE));
	V(theApp.m_d3dDevice->SetRenderState(D3DRS_LIGHTING, FALSE));
	V(theApp.m_d3dDevice->SetRenderState(D3DRS_FILLMODE, D3DFILL_SOLID));
	V(theApp.m_d3dDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_CW));
	V(theApp.m_d3dDevice->SetTransform(D3DTS_VIEW, (D3DMATRIX*)&rc.m_Camera->m_View));
	V(theApp.m_d3dDevice->SetTransform(D3DTS_PROJECTION, (D3DMATRIX*)&rc.m_Camera->m_Proj));
	V(theApp.m_d3dDevice->SetTransform(D3DTS_WORLD, (D3DMATRIX*)&my::Matrix4::identity));
	DrawHelper::FlushLine(theApp.m_d3dDevice);

	CString ext(PathFindExtension(m_TexPath));
	V(D3DXSaveTextureToFile(m_TexPath, ext.CompareNoCase(_T(".dds")) == 0 ? D3DXIFF_DDS : D3DXIFF_PNG, rt.m_ptr, NULL));
}
