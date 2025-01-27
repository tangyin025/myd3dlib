// Copyright (c) 2011-2024 tangyin025
// License: MIT

#include "stdafx.h"
#include "StaticEmitterDlg.h"
#include "afxdialogex.h"
#include "Material.h"
#include "MainApp.h"


// CStaticEmitterDlg dialog

IMPLEMENT_DYNAMIC(CStaticEmitterDlg, CDialogEx)

CStaticEmitterDlg::CStaticEmitterDlg(const char* StaticEmitterName, CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_DIALOG5, pParent)
	, m_emit_cmp_name(StaticEmitterName)
	, m_ChunkWidth(4.0f)
	, m_ChunkLodScale(1.0f)
{
	m_AssetPath.Format(_T("terrain/%s"), ms2ts(m_emit_cmp_name.c_str()).c_str());
}

CStaticEmitterDlg::~CStaticEmitterDlg()
{
}

void CStaticEmitterDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT1, m_AssetPath);
	DDX_Text(pDX, IDC_EDIT2, m_BoundingBox.m_min.x);
	DDX_Text(pDX, IDC_EDIT3, m_BoundingBox.m_min.y);
	DDX_Text(pDX, IDC_EDIT4, m_BoundingBox.m_min.z);
	DDX_Text(pDX, IDC_EDIT5, m_BoundingBox.m_max.x);
	DDX_Text(pDX, IDC_EDIT6, m_BoundingBox.m_max.y);
	DDX_Text(pDX, IDC_EDIT7, m_BoundingBox.m_max.z);
	DDX_Text(pDX, IDC_EDIT8, m_ChunkWidth);
	DDV_MinMaxFloat(pDX, m_ChunkWidth, EPSILON_E3, 1024);
	DDX_Text(pDX, IDC_EDIT9, m_ChunkLodScale);
	DDV_MinMaxFloat(pDX, m_ChunkLodScale, EPSILON_E6, FLT_MAX);
}


BEGIN_MESSAGE_MAP(CStaticEmitterDlg, CDialogEx)
	ON_EN_CHANGE(IDC_EDIT1, &CStaticEmitterDlg::OnChangeEdit1)
END_MESSAGE_MAP()


// CStaticEmitterDlg message handlers

BOOL CStaticEmitterDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// TODO:  Add extra initialization here
	OnChangeEdit1();

	return TRUE;  // return TRUE unless you set the focus to a control
				  // EXCEPTION: OCX Property Pages should return FALSE
}


void CStaticEmitterDlg::OnOK()
{
	// TODO: Add your specialized code here and/or call the base class
	if (!UpdateData(TRUE))
	{
		TRACE(traceAppMsg, 0, "UpdateData failed during dialog termination.\n");
		// the UpdateData routine will set focus to correct item
		return;
	}

	CString strText;
	GetDlgItemText(IDC_STATIC5, strText);
	if (!strText.IsEmpty())
	{
		strText.Format(_T("Overwrite existed '%s_*'?"), m_AssetPath);
		if (IDCANCEL == AfxMessageBox(strText, MB_OKCANCEL))
		{
			return;
		}

		std::basic_string<TCHAR> FullPath = theApp.GetFullPath(ts2ms((LPCTSTR)m_AssetPath).c_str());
		FullPath.append(_T("_*\0"), 3);
		SHFILEOPSTRUCT shfo;
		ZeroMemory(&shfo, sizeof(shfo));
		shfo.hwnd = AfxGetMainWnd()->m_hWnd;
		shfo.wFunc = FO_DELETE;
		shfo.pFrom = FullPath.c_str();
		shfo.fFlags = FOF_ALLOWUNDO | FOF_FILESONLY | FOF_NOCONFIRMATION | FOF_NORECURSION;
		int res = SHFileOperation(&shfo);
		if (res != 0)
		{
			return;
		}
	}

	m_emit_cmp.reset(new StaticEmitter(m_emit_cmp_name.c_str(),
		m_BoundingBox, m_ChunkWidth, EmitterComponent::FaceTypeCamera, EmitterComponent::SpaceTypeLocal));

	m_emit_cmp->m_ChunkLodScale = m_ChunkLodScale;

	m_emit_cmp->m_ChunkPath = ts2ms((LPCTSTR)m_AssetPath);

	StaticEmitterStream estr(m_emit_cmp.get());
	estr.Spawn(my::Vector4(0, 0, 0, 1), my::Vector4(0, 0, 0, 1), my::Vector4(1, 1, 1, 1), my::Vector2(10, 10), 0.0f, 0.0f);
	estr.Flush();

	{
		MaterialPtr mtl(new Material());
		mtl->m_Shader = theApp.default_shader;
		mtl->ParseShaderParameters();
		m_emit_cmp->SetMaterial(mtl);
	}

	EndDialog(IDOK);
}


void CStaticEmitterDlg::OnChangeEdit1()
{
	// TODO:  If this is a RICHEDIT control, the control will not
	// send this notification unless you override the CDialogEx::OnInitDialog()
	// function and call CRichEditCtrl().SetEventMask()
	// with the ENM_CHANGE flag ORed into the mask.

	// TODO:  Add your control notification handler code here
	CString strText;
	GetDlgItemText(IDC_EDIT1, strText);
	std::basic_string<TCHAR> FullPath = theApp.GetFullPath(ts2ms((LPCTSTR)strText).c_str()) + _T("_*");
	WIN32_FIND_DATA data;
	HANDLE h = FindFirstFile(FullPath.c_str(), &data);
	if (h == INVALID_HANDLE_VALUE)
	{
		SetDlgItemText(IDC_STATIC5, _T(""));
		return;
	}
	SetDlgItemText(IDC_STATIC5, _T("Existed !"));
	FindClose(h);
}
