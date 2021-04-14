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

	m_terrain.reset(new Terrain(m_terrain_name.c_str(), m_RowChunks, m_ColChunks, m_ChunkSize, 1.0f));

	m_terrain->m_ChunkPath = ts2ms(m_ChunkPath);

	TerrainStream tstr(m_terrain.get());
	std::fill(tstr.m_AabbDirty.data(), tstr.m_AabbDirty.data() + tstr.m_AabbDirty.num_elements(), true);
	tstr.Release();

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
	if (my::ResourceMgr::getSingleton().CheckPath(TerrainStream::GetChunkPath(ts2ms(strText).c_str(), 0, 0).c_str()))
	{
		SetDlgItemText(IDC_STATIC5, _T("Existed !"));
	}
	else
	{
		SetDlgItemText(IDC_STATIC5, _T(""));
	}
}
