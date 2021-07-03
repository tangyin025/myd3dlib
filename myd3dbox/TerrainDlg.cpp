// TerrainDlg.cpp : implementation file
//

#include "stdafx.h"
#include "MainApp.h"
#include "TerrainDlg.h"
#include "afxdialogex.h"
#include "CtrlProps.h"
#include "Component.h"
#include "PropertiesWnd.h"

// CTerrainDlg dialog

IMPLEMENT_DYNAMIC(CTerrainDlg, CDialogEx)

CTerrainDlg::CTerrainDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CTerrainDlg::IDD, pParent)
	, m_terrain_name(my::NamedObject::MakeUniqueName("editor_terrain"))
	, m_RowChunks(1)
	, m_ColChunks(1)
	, m_ChunkSize(32)
	, m_AlignToCenter(TRUE)
	, m_MinLodChunkSize(2)
{
	m_ChunkPath.Format(_T("terrain/%s"), ms2ts(m_terrain_name.c_str()).c_str());
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
	DDX_Text(pDX, IDC_EDIT4, m_ChunkPath);
	DDX_Check(pDX, IDC_CHECK1, m_AlignToCenter);
	DDX_Text(pDX, IDC_EDIT5, m_MinLodChunkSize);
}


BEGIN_MESSAGE_MAP(CTerrainDlg, CDialogEx)
	ON_EN_CHANGE(IDC_EDIT4, &CTerrainDlg::OnChangeEdit4)
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
		strText.Format(_T("Overwrite existed '%s'?"), m_ChunkPath);
		if (IDCANCEL == AfxMessageBox(strText, MB_OKCANCEL))
		{
			return;
		}
	}

	if (!strText.IsEmpty() && !::DeleteFileA(theApp.GetFullPath(ts2ms(m_ChunkPath).c_str()).c_str()))
	{
		::FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, NULL, GetLastError(),
			MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US), strText.GetBufferSetLength(MAX_PATH), strText.GetAllocLength(), NULL);
		AfxMessageBox(strText, MB_OK);
		return;
	}

	m_terrain.reset(new Terrain(m_terrain_name.c_str(), m_RowChunks, m_ColChunks, m_ChunkSize, m_MinLodChunkSize, 512.0f / SHRT_MAX));

	m_terrain->m_ChunkPath = ts2ms(m_ChunkPath);

	TerrainStream tstr(m_terrain.get());
	tstr.GetPos(0, 0);

	{
		MaterialPtr mtl(new Material());
		mtl->m_Shader = theApp.default_shader;
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
	if (my::ResourceMgr::getSingleton().CheckPath(theApp.GetFullPath(ts2ms(strText).c_str()).c_str()))
	{
		SetDlgItemText(IDC_STATIC5, _T("Existed !"));
	}
	else
	{
		SetDlgItemText(IDC_STATIC5, _T(""));
	}
}
