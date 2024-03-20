// CSnapshotDlg.cpp : implementation file
//

#include "stdafx.h"
#include "SnapshotDlg.h"
#include "afxdialogex.h"
#include "MainApp.h"
#include "MainFrm.h"
#include "Actor.h"
#include "NavigationSerialization.h"


// CSnapshotDlg dialog

IMPLEMENT_DYNAMIC(CSnapshotDlg, CDialogEx)

CSnapshotDlg::CSnapshotDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_DIALOG7, pParent)
	, m_TexPath(theApp.GetProfileString(_T("Settings"), _T("SnapshotPath"), _T("aaa.png")))
	, m_TexWidth(theApp.GetProfileInt(_T("Settings"), _T("SnapshotWidth"), 1024))
	, m_TexHeight(theApp.GetProfileInt(_T("Settings"), _T("SnapshotHeight"), 1024))
	, m_duDebugDrawPrimitives(DU_DRAW_QUADS + 1)
	, m_SnapArea(-4096 + 4, -4096 - 4, 4096 + 4, 4096 - 4)
	, m_SnapEye(0, 0, 0)
	, m_SnapEular(-90, 0, 0)
	, m_RTType(RenderPipeline::RenderTargetOpaque)
{
	BYTE* pData;
	UINT n;
	if (theApp.GetProfileBinary(_T("Settings"), _T("SnapshotArea"), &pData, &n))
	{
		ASSERT(n == sizeof(m_SnapArea));
		m_SnapArea = *(my::Rectangle*)pData;
		delete[] pData; // free the buffer
	}
	if (theApp.GetProfileBinary(_T("Settings"), _T("SnapshotEye"), &pData, &n))
	{
		ASSERT(n == sizeof(m_SnapEye));
		m_SnapEye = *(my::Vector3*)pData;
		delete[] pData;
	}
	if (theApp.GetProfileBinary(_T("Settings"), _T("SnapshotEular"), &pData, &n))
	{
		ASSERT(n == sizeof(m_SnapEular));
		m_SnapEular = *(my::Vector3*)pData;
		delete[] pData;
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
	DDX_Text(pDX, IDC_EDIT11, m_SnapEular.x);
	DDX_Text(pDX, IDC_EDIT12, m_SnapEular.y);
	DDX_Text(pDX, IDC_EDIT13, m_SnapEular.z);
	DDX_Check(pDX, IDC_CHECK1, m_ComponentTypes[0]);
	DDX_Check(pDX, IDC_CHECK2, m_ComponentTypes[1]);
	DDX_Check(pDX, IDC_CHECK3, m_ComponentTypes[2]);
	DDX_Check(pDX, IDC_CHECK4, m_ComponentTypes[3]);
	DDX_Check(pDX, IDC_CHECK5, m_ComponentTypes[4]);
	DDX_Check(pDX, IDC_CHECK6, m_ComponentTypes[5]);
	DDX_Check(pDX, IDC_CHECK7, m_ComponentTypes[6]);
	DDX_Check(pDX, IDC_CHECK8, m_ComponentTypes[7]);
	DDX_Check(pDX, IDC_CHECK9, m_ComponentTypes[8]);

	if (pDX->m_bSaveAndValidate)
	{
		theApp.WriteProfileString(_T("Settings"), _T("SnapshotPath"), m_TexPath);
		theApp.WriteProfileInt(_T("Settings"), _T("SnapshotWidth"), m_TexWidth);
		theApp.WriteProfileInt(_T("Settings"), _T("SnapshotHeight"), m_TexHeight);
		theApp.WriteProfileBinary(_T("Settings"), _T("SnapshotArea"), (LPBYTE)&m_SnapArea, sizeof(m_SnapArea));
		theApp.WriteProfileBinary(_T("Settings"), _T("SnapshotEye"), (LPBYTE)&m_SnapEye, sizeof(m_SnapEye));
		theApp.WriteProfileBinary(_T("Settings"), _T("SnapshotEular"), (LPBYTE)&m_SnapEular, sizeof(m_SnapEular));
		theApp.WriteProfileBinary(_T("Settings"), _T("SnapshotComponentTypes"), (LPBYTE)&m_ComponentTypes, sizeof(m_ComponentTypes));
	}
	DDX_Radio(pDX, IDC_RADIO1, m_RTType);
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
				const my::Vector3& ViewPos;
				const my::Vector3& TargetPos;
				CMainFrame* pFrame;
				CSnapshotDlg* pDlg;

				Callback(const my::Frustum& _frustum, RenderPipeline* _pipeline, unsigned int _PassMask, const my::Vector3& _ViewPos, const my::Vector3& _TargetPos, CMainFrame* _pFrame, CSnapshotDlg* _pDlg)
					: frustum(_frustum)
					, pipeline(_pipeline)
					, PassMask(_PassMask)
					, ViewPos(_ViewPos)
					, TargetPos(_TargetPos)
					, pFrame(_pFrame)
					, pDlg(_pDlg)
				{
				}

				virtual bool OnQueryEntity(my::OctEntity* oct_entity, const my::AABB& aabb, my::IntersectionTests::IntersectionType)
				{
					ASSERT(dynamic_cast<Actor*>(oct_entity));

					Actor* actor = static_cast<Actor*>(oct_entity);

					if (!actor->IsRequested())
					{
						_ASSERT(!actor->is_linked());

						actor->RequestResource();

						pFrame->m_ViewedActors.push_back(*actor);
					}

					Actor::ComponentPtrList::iterator cmp_iter = actor->m_Cmps.begin();
					for (; cmp_iter != actor->m_Cmps.end(); cmp_iter++)
					{
						if ((*cmp_iter)->GetComponentType() >= Component::ComponentTypeMesh
							&& (*cmp_iter)->GetComponentType() <= Component::ComponentTypeNavigation
							&& pDlg->m_ComponentTypes[(*cmp_iter)->GetComponentType() - Component::ComponentTypeMesh]
							&& (*cmp_iter)->m_LodMask & Component::LOD0)
						{
							if (!(*cmp_iter)->IsRequested())
							{
								(*cmp_iter)->RequestResource();

								theApp.LeaveDeviceSection();
								theApp.CheckIORequests(0xFFFFFFFF); // ! INFINITE conflict with corecrt_math.h
								theApp.EnterDeviceSection();
							}

							if ((*cmp_iter)->GetComponentType() == Component::ComponentTypeNavigation
								&& PassMask & RenderPipeline::PassTypeToMask(RenderPipeline::PassTypeNormal))
							{
								Navigation* navi = dynamic_cast<Navigation*>(cmp_iter->get());
								navi->DebugDraw(pDlg, frustum, ViewPos, TargetPos);
							}
							else
							{
								(*cmp_iter)->AddToPipeline(frustum, pipeline, PassMask, ViewPos, TargetPos);
							}
						}
					}
					return true;
				}
			};

			CMainFrame* pFrame = DYNAMIC_DOWNCAST(CMainFrame, AfxGetMainWnd());
			ASSERT_VALID(pFrame);
			Callback cb(frustum, pipeline, PassMask, m_Camera->m_Eye, m_Camera->m_Eye, pFrame, pDlg);
			pFrame->QueryEntity(frustum, &cb);
		}
	};

	RenderContext rc(this);
	rc.m_Camera.reset(new my::OrthoCamera(m_SnapArea.Width(), m_SnapArea.Height(), -2000, 2000));
	my::OrthoCamera* ortho_camera = dynamic_cast<my::OrthoCamera*>(rc.m_Camera.get());
	ortho_camera->m_Eye = m_SnapEye;
	ortho_camera->m_Euler = my::Vector3(D3DXToRadian(m_SnapEular.x), D3DXToRadian(m_SnapEular.y), D3DXToRadian(m_SnapEular.z));
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
