
#pragma once

class CMyPropertyGridFontProperty : public CMFCPropertyGridFontProperty
{
public:
	CMyPropertyGridFontProperty(	const CString& strName, LOGFONT& lf, DWORD dwFontDialogFlags = CF_EFFECTS | CF_SCREENFONTS, 
		LPCTSTR lpszDescr = NULL, DWORD_PTR dwData = 0, COLORREF color = (COLORREF)-1)
		: CMFCPropertyGridFontProperty(strName, lf, dwFontDialogFlags, lpszDescr, dwData, color)
	{
	}

	void SetLogFont(LOGFONT & lf);
};

class CPropertiesWnd : public CDockablePane
{
public:
	CPropertiesWnd();

	void AdjustLayout();

	CFont m_fntPropList;

	CMFCPropertyGridCtrl m_wndPropList;

	enum Property
	{
		PropertyGroupCoord = 0,
		PropertyGroupImage,
		PropertyGroupFont,
		PropertyItemLocal,
		PropertyItemLocalX,
		PropertyItemLocalY,
		PropertyItemSize,
		PropertyItemSizeW,
		PropertyItemSizeH,
		PropertyItemAlpha,
		PropertyItemRGB,
		PropertyItemImage,
		PropertyItemBorder,
		PropertyItemBorderX,
		PropertyItemBorderY,
		PropertyItemBorderZ,
		PropertyItemBorderW,
		PropertyItemFont,
		PropertyItemFontSize,
		PropertyItemFontAlpha,
		PropertyItemFontRGB,
		PropertyCount
	};

	CMFCPropertyGridProperty * m_pProp[PropertyCount];

protected:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);

	afx_msg void OnSize(UINT nType, int cx, int cy);

	afx_msg void OnSetFocus(CWnd* pOldWnd);

	afx_msg void OnSettingChange(UINT uFlags, LPCTSTR lpszSection);

	DECLARE_MESSAGE_MAP()

	void SetPropListFont();

public:
	void OnIdleUpdate();

	void UpdateProperties(void);

	afx_msg LRESULT OnPropertyChanged(WPARAM wParam, LPARAM lParam);
};

