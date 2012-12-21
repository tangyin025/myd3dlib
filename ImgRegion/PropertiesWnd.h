
#pragma once

class CFileProp : public CMFCPropertyGridFileProperty
{
public:
	CFileProp(const CString& strName, BOOL bOpenFileDialog, const CString& strFileName, LPCTSTR lpszDefExt = NULL,
		DWORD dwFlags = OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, LPCTSTR lpszFilter = NULL, LPCTSTR lpszDescr = NULL, DWORD_PTR dwData = 0)
		: CMFCPropertyGridFileProperty(strName, bOpenFileDialog, strFileName, lpszDefExt, dwFlags, lpszFilter, lpszDescr, dwData)
	{
	}

	virtual void OnClickButton(CPoint point);
};

class CSliderProp : public CMFCPropertyGridProperty
{
public:
	CSliderProp(const CString& strName, const COleVariant& varValue, LPCTSTR lpszDescr = NULL, DWORD_PTR dwData = 0)
		: CMFCPropertyGridProperty(strName, varValue, lpszDescr, dwData)
	{
		m_dwFlags = 0x0002;
	}

	virtual void OnClickButton(CPoint point);
};

class CCheckBoxProp : public CMFCPropertyGridProperty
{
public:
	CCheckBoxProp(const CString& strName, BOOL bCheck, LPCTSTR lpszDescr = NULL, DWORD dwData = 0);

protected:
	virtual BOOL OnEdit(LPPOINT /*lptClick*/) { return FALSE; }
	virtual void OnDrawButton(CDC* /*pDC*/, CRect /*rectButton*/) {}
	virtual void OnDrawValue(CDC* /*pDC*/, CRect /*rect*/) {}
	virtual BOOL HasButton() const { return FALSE; }

	virtual BOOL PushChar(UINT nChar);
	virtual void OnDrawCheckBox(CDC * pDC, CRect rectCheck, BOOL bChecked);
	virtual void OnDrawName(CDC* pDC, CRect rect);
	virtual void OnClickName(CPoint point);
	virtual BOOL OnDblClk(CPoint point);

protected:
	CRect m_rectCheck;
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
		PropertyItemLocked,
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
		PropertyItemTextOff,
		PropertyItemTextOffX,
		PropertyItemTextOffY,
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

