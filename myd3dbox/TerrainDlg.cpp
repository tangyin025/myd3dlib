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

CTerrainDlg::CTerrainDlg(const char * TerrainComponentName, CWnd* pParent /*=NULL*/)
	: CDialogEx(CTerrainDlg::IDD, pParent)
	, m_terrain_name(TerrainComponentName)
	, m_RowChunks(1)
	, m_ColChunks(1)
	, m_ChunkSize(64)
	, m_AlignToCenter(TRUE)
	, m_MinLodChunkSize(2)
	, m_ActorScale(0)
{
	m_AssetPath.Format(_T("terrain/%s"), ms2ts(m_terrain_name).c_str());
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
	DDX_Text(pDX, IDC_EDIT5, m_MinLodChunkSize);
	DDX_Text(pDX, IDC_EDIT6, m_ActorScale.x);
	DDV_MinMaxFloat(pDX, m_ActorScale.x, EPSILON_E6, FLT_MAX);
	DDX_Text(pDX, IDC_EDIT7, m_ActorScale.y);
	DDV_MinMaxFloat(pDX, m_ActorScale.y, EPSILON_E6, FLT_MAX);
	DDX_Text(pDX, IDC_EDIT8, m_ActorScale.z);
	DDV_MinMaxFloat(pDX, m_ActorScale.z, EPSILON_E6, FLT_MAX);
}


BEGIN_MESSAGE_MAP(CTerrainDlg, CDialogEx)
	ON_EN_CHANGE(IDC_EDIT4, &CTerrainDlg::OnChangeEdit4)
	ON_BN_CLICKED(IDC_BUTTON1, &CTerrainDlg::OnClickedButton1)
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

	if (m_AssetPath.IsEmpty())
	{
		MessageBox(_T("m_AssetPath.IsEmpty()"));
		return;
	}

	m_terrain.reset(new Terrain(m_terrain_name.c_str(), m_RowChunks, m_ColChunks, m_ChunkSize, m_MinLodChunkSize));

	m_terrain->m_ChunkPath = ts2ms((LPCTSTR)m_AssetPath);


	if (!GetDlgItem(IDC_BUTTON1)->IsWindowEnabled())
	{
		TerrainStream tstr(m_terrain.get());
		tstr.GetPos(0, 0);
		tstr.Release();
	}

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
	if (my::ResourceMgr::getSingleton().CheckPath(theApp.GetFullPath(ts2ms((LPCTSTR)strText).c_str()).c_str()))
	{
		GetDlgItem(IDC_BUTTON1)->EnableWindow(TRUE);
	}
	else
	{
		GetDlgItem(IDC_BUTTON1)->EnableWindow(FALSE);
	}
}


void CTerrainDlg::OnClickedButton1()
{
	// TODO: Add your control notification handler code here
	if (!UpdateData(TRUE))
	{
		TRACE(traceAppMsg, 0, "UpdateData failed during dialog termination.\n");
		// the UpdateData routine will set focus to correct item
		return;
	}

	std::string FullPath = theApp.GetFullPath(ts2ms((LPCTSTR)m_AssetPath).c_str());
	FullPath.append(2, '\0');
	SHFILEOPSTRUCTA shfo;
	ZeroMemory(&shfo, sizeof(shfo));
	shfo.hwnd = AfxGetMainWnd()->m_hWnd;
	shfo.wFunc = FO_DELETE;
	shfo.pFrom = FullPath.c_str();
	shfo.fFlags = FOF_ALLOWUNDO | FOF_FILESONLY | FOF_NOCONFIRMATION | FOF_NORECURSION;
	int res = SHFileOperationA(&shfo);
	if (res != 0)
	{
		MessageBox(str_printf(_T("SHFileOperation failed: %s"), ms2ts(FullPath).c_str()).c_str());
	}

	OnChangeEdit4();
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
}
