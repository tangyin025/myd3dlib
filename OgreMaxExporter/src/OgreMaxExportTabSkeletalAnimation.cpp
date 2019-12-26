#include "OgreExport.h"
#include "resource.h"

static OgreMaxExport_SkeletalAnimation* sInst;

void OgreMaxExport_SkeletalAnimation::onInitDialog(HWND hDlg) {
	OgreMaxExport_TabPaneHandler::onInitDialog(hDlg);

	// set up animation listbox
	int frameStart = m_i->GetAnimRange().Start();
	int frameEnd = m_i->GetAnimRange().End();

	HWND anims = GetDlgItem(m_hDlg, IDC_LIST_ANIMATIONS);

	Rect r;
	GetWindowRect(anims, &r);

	LVCOLUMN lvc;
	ZeroMemory(&lvc, sizeof(LVCOLUMN));
	lvc.mask = LVCF_TEXT | LVCF_WIDTH;
	lvc.cx = r.w() * 0.6;
	lvc.pszText = _T("Animation");
	ListView_InsertColumn(anims, 0, &lvc);
	lvc.cx = r.w() * 0.2;
	lvc.pszText = _T("Begin");
	ListView_InsertColumn(anims, 1, &lvc);
	lvc.pszText = _T("End");
	ListView_InsertColumn(anims, 2, &lvc);

	// add a spanning entry to the animation list as a default
	LVITEM lvi;
	TCHAR buf[32];
	ZeroMemory(&lvi, sizeof(LVITEM));

	lvi.mask = LVIF_TEXT;
	lvi.pszText = _T("Animation");
	lvi.iItem = 10000;
	int idx = ListView_InsertItem(anims, &lvi);

	_stprintf(buf, _T("%d"), frameStart / GetTicksPerFrame());
	lvi.iItem = idx;
	lvi.iSubItem = 1;
	lvi.pszText = buf;
	ListView_SetItem(anims, &lvi);

	_stprintf(buf, _T("%d"), frameEnd / GetTicksPerFrame());
	lvi.iSubItem = 2;
	lvi.pszText = buf;
	ListView_SetItem(anims, &lvi);

	// populate the frame range info box
	_stprintf(buf, _T("%d to %d"), frameStart / GetTicksPerFrame(), frameEnd / GetTicksPerFrame());
	SendMessage(GetDlgItem(m_hDlg, IDC_TXT_FRAME_RANGE), WM_SETTEXT, 0, (LPARAM)buf);
	SendMessage(GetDlgItem(m_hDlg, IDC_TXT_FPS), WM_SETTEXT, 0, (LPARAM)_T("1.0"));
}

void OgreMaxExport_SkeletalAnimation::onDestroy() {
	update();
}

// read the contents from the dialog controls
void OgreMaxExport_SkeletalAnimation::update() {
	HWND anims = GetDlgItem(m_hDlg, IDC_LIST_ANIMATIONS);
	LVITEM lvi;
	TCHAR buf[256];
	ZeroMemory(&lvi, sizeof(LVITEM));
	m_exp->m_meshXMLExporter.m_animations.clear();
	int count = ListView_GetItemCount(anims);
	for (int i = 0; i < count; i++) {
		lvi.mask = LVIF_TEXT;
		lvi.iItem = i;
		lvi.iSubItem = 0;
		lvi.pszText = buf;
		lvi.cchTextMax = _countof(buf);
		ListView_GetItem(anims, &lvi);

		OgreMax::MeshXMLExporter::NamedAnimation anim;
		anim.name = std::basic_string<TCHAR>(lvi.pszText);

		lvi.iSubItem = 1;
		ListView_GetItem(anims, &lvi);
		anim.start = _tstoi(lvi.pszText);

		lvi.iSubItem = 2;
		ListView_GetItem(anims, &lvi);
		anim.end = _tstoi(lvi.pszText);

		m_exp->m_meshXMLExporter.m_animations.push_back(anim);
	}
}

void OgreMaxExport_SkeletalAnimation::onAddAnimation() {
	addAnimation();
}

void OgreMaxExport_SkeletalAnimation::onDeleteAnimation() {
	deleteAnimation();
}

void OgreMaxExport_SkeletalAnimation::addAnimation() {
	TCHAR buf[256];
	int start, end;
	HWND anims = GetDlgItem(m_hDlg, IDC_LIST_ANIMATIONS);

	SendMessage(GetDlgItem(m_hDlg, IDC_TXT_FPS), WM_GETTEXT, 256, (LPARAM)buf);
	float fps = _tstof(buf);

	if (fps <= 0.0) {
		MessageBox(NULL, _T("FPS must be >= 0.0"), _T("Invalid Entry"), MB_ICONEXCLAMATION);
		return;
	}

	int minAnimTime = m_i->GetAnimRange().Start() / GetTicksPerFrame();
	int maxAnimTime = m_i->GetAnimRange().End() / GetTicksPerFrame();

	// get animation start and end times
	SendMessage(GetDlgItem(m_hDlg, IDC_TXT_ANIM_START), WM_GETTEXT, 256, (LPARAM)buf);
	start = _tstoi(buf);

	if (start < minAnimTime) {
		_stprintf(buf, _T("Start time must be >= %d"), start);
		MessageBox(NULL, buf, _T("Invalid Entry"), MB_ICONEXCLAMATION);
		return;
	}

	SendMessage(GetDlgItem(m_hDlg, IDC_TXT_ANIM_END), WM_GETTEXT, 256, (LPARAM)buf);
	end = _tstoi(buf);

	if (end > maxAnimTime) {
		_stprintf(buf, _T("End time must be <= %d"), end);
		MessageBox(NULL, buf, _T("Invalid Entry"), MB_ICONEXCLAMATION);
		return;
	}

	// get animation name
	SendMessage(GetDlgItem(m_hDlg, IDC_TXT_ANIMATION_NAME), WM_GETTEXT, 256, (LPARAM)buf);
	std::basic_string<TCHAR> name(buf);

	if (name.length() == 0) {
		MessageBox(NULL, _T("Animation name must not be empty"), _T("Invalid Entry"), MB_ICONEXCLAMATION);
		return;
	}

	// if, after all that, we have valid data, stick it in the listview
	LVITEM lvi;
	ZeroMemory(&lvi, sizeof(LVITEM));

	lvi.mask = LVIF_TEXT;
	lvi.pszText = buf;
	lvi.iItem = 10000;
	int idx = ListView_InsertItem(anims, &lvi);

	lvi.iItem = idx;
	lvi.iSubItem = 1;
	_stprintf(buf, _T("%d"), start);
	lvi.pszText = buf;
	ListView_SetItem(anims, &lvi);
	lvi.iSubItem = 2;
	_stprintf(buf, _T("%d"), end);
	lvi.pszText = buf;
	ListView_SetItem(anims, &lvi);

	// Finally, clear out the entry controls
	SetWindowText(GetDlgItem(m_hDlg, IDC_TXT_ANIMATION_NAME), _T(""));
	SetWindowText(GetDlgItem(m_hDlg, IDC_TXT_ANIM_START), _T(""));
	SetWindowText(GetDlgItem(m_hDlg, IDC_TXT_ANIM_END), _T(""));
}

void OgreMaxExport_SkeletalAnimation::deleteAnimation() {

	HWND anims = GetDlgItem(m_hDlg, IDC_LIST_ANIMATIONS);

	// delete selected animation(s) from the listview
	int idx;
	while ((idx=ListView_GetNextItem(anims, -1, LVNI_SELECTED)) != -1)
		ListView_DeleteItem(anims, idx);
}

// for the sake of sanity, keep the dlgproc and the handler class implementation here in the same source file
INT_PTR CALLBACK SkeletalAnimationTabDialogProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam) {

	switch(message) {
	case WM_INITDIALOG:
		sInst = (OgreMaxExport_SkeletalAnimation*) lParam;

		sInst->onInitDialog(hDlg);
		SetWindowPos(hDlg, HWND_TOP, 10, 40, 0, 0, SWP_NOSIZE);
		ShowWindow(hDlg, SW_SHOW);
		break;

	case WM_COMMAND:
		switch(LOWORD(wParam)) {
		case IDC_CMD_DELETE_ANIMATION:
			sInst->onDeleteAnimation();
			break;
		case IDC_CMD_ADD_ANIMATION:
			sInst->onAddAnimation();
			break;
		}
		break;

	case WM_NOTIFY:
		break;

	case WM_DESTROY:
		sInst->onDestroy();
		break;
	}
	return FALSE;
}
