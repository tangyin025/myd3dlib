// Copyright (c) 2011-2024 tangyin025
// License: MIT

#include "stdafx.h"
#include "MainApp.h"
#include "TerrainDlg.h"
#include "afxdialogex.h"
#include "CtrlProps.h"
#include "Component.h"
#include "Material.h"
#include "PropertiesWnd.h"
#include <boost/lexical_cast.hpp>

// CTerrainDlg dialog

IMPLEMENT_DYNAMIC(CTerrainDlg, CDialogEx)

CTerrainDlg::CTerrainDlg(const char * TerrainComponentName, CWnd* pParent /*=NULL*/)
	: CDialogEx(CTerrainDlg::IDD, pParent)
	, m_terrain_name(TerrainComponentName)
	, m_RowChunks(1)
	, m_ColChunks(1)
	, m_ChunkSize(64)
	, m_AlignToCenter(TRUE)
	, m_MinChunkLodSize(2)
	, m_ActorScale(1.0f)
	, m_ChunkLodScale(1.0f)
{
	m_AssetPath.Format(_T("terrain/%s"), ms2ts(m_terrain_name.c_str()).c_str());
}

CTerrainDlg::~CTerrainDlg()
{
}

void CTerrainDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT1, m_RowChunks);
	DDX_Text(pDX, IDC_EDIT2, m_ColChunks);
	DDX_Text(pDX, IDC_EDIT3, m_ChunkSize);
	DDX_Text(pDX, IDC_EDIT4, m_AssetPath);
	DDX_Check(pDX, IDC_CHECK1, m_AlignToCenter);
	DDX_Text(pDX, IDC_EDIT5, m_MinChunkLodSize);
	DDX_Text(pDX, IDC_EDIT6, m_ActorScale.x);
	DDV_MinMaxFloat(pDX, m_ActorScale.x, EPSILON_E12, FLT_MAX);
	DDX_Text(pDX, IDC_EDIT7, m_ActorScale.y);
	DDV_MinMaxFloat(pDX, m_ActorScale.y, EPSILON_E12, FLT_MAX);
	DDX_Text(pDX, IDC_EDIT8, m_ActorScale.z);
	DDV_MinMaxFloat(pDX, m_ActorScale.z, EPSILON_E12, FLT_MAX);
	DDX_Text(pDX, IDC_EDIT9, m_ChunkLodScale);
	DDV_MinMaxFloat(pDX, m_ChunkLodScale, EPSILON_E12, FLT_MAX);
}


BEGIN_MESSAGE_MAP(CTerrainDlg, CDialogEx)
	ON_EN_CHANGE(IDC_EDIT4, &CTerrainDlg::OnChangeEdit4)
	ON_EN_CHANGE(IDC_EDIT6, &CTerrainDlg::OnChangeEdit6)
END_MESSAGE_MAP()


// CTerrainDlg message handlers


BOOL CTerrainDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// TODO:  Add extra initialization here
	OnChangeEdit4();

	return TRUE;  // return TRUE unless you set the focus to a control
				  // EXCEPTION: OCX Property Pages should return FALSE
}

void CTerrainDlg::OnOK()
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

		std::basic_string<TCHAR> FullPath = theApp.GetFullPath(ts2ms((LPCTSTR)m_AssetPath).c_str()) + _T("_*");
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

	m_terrain.reset(new Terrain(m_terrain_name.c_str(), m_RowChunks, m_ColChunks, m_ChunkSize, m_MinChunkLodSize));

	m_terrain->m_ChunkPath = ts2ms((LPCTSTR)m_AssetPath);

	m_terrain->m_ChunkLodScale = m_ChunkLodScale;

	TerrainStream tstr(m_terrain.get());
	tstr.GetPos(0, 0);
	tstr.Flush();

	{
		MaterialPtr mtl(new Material());
		mtl->m_Shader = theApp.default_terrain_shader;
		mtl->ParseShaderParameters();
		m_terrain->SetMaterial(mtl);
	}

	EndDialog(IDOK);
}


void CTerrainDlg::OnChangeEdit4()
{
	// TODO:  If this is a RICHEDIT control, the control will not
	// send this notification unless you override the CDialogEx::OnInitDialog()
	// function and call CRichEditCtrl().SetEventMask()
	// with the ENM_CHANGE flag ORed into the mask.

	// TODO:  Add your control notification handler code here
	CString strText;
	GetDlgItemText(IDC_EDIT4, strText);
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


void CTerrainDlg::OnChangeEdit6()
{
	// TODO:  If this is a RICHEDIT control, the control will not
	// send this notification unless you override the CDialogEx::OnInitDialog()
	// function and call CRichEditCtrl().SetEventMask()
	// with the ENM_CHANGE flag ORed into the mask.

	// TODO:  Add your control notification handler code here
	CString strText;
	GetDlgItemText(IDC_EDIT6, strText);
	SetDlgItemText(IDC_EDIT7, strText);
	SetDlgItemText(IDC_EDIT8, strText);
	float scale = boost::lexical_cast<float>((LPCTSTR)strText);
	SetDlgItemText(IDC_EDIT9, boost::lexical_cast<std::basic_string<TCHAR> >(1 / scale).c_str());
}
