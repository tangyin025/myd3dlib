
#pragma once

class CImgRegionPropertyGridFileProperty : public CMFCPropertyGridFileProperty
{
public:
	CImgRegionPropertyGridFileProperty(const CString& strName, BOOL bOpenFileDialog, const CString& strFileName, LPCTSTR lpszDefExt = NULL,
		DWORD dwFlags = OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, LPCTSTR lpszFilter = NULL, LPCTSTR lpszDescr = NULL, DWORD_PTR dwData = 0)
		: CMFCPropertyGridFileProperty(strName, bOpenFileDialog, strFileName, lpszDefExt, dwFlags, lpszFilter, lpszDescr, dwData)
	{
	}

	virtual void OnClickButton(CPoint point);
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
		PropertyGroupText,
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
		PropertyItemText,
		PropertyCount
	};

	CMFCPropertyGridProperty * m_pProp[PropertyCount];

	BOOL m_bIsPropInvalid;

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

	void InvalidProperties(void)
	{
		m_bIsPropInvalid = TRUE;
	}
};

