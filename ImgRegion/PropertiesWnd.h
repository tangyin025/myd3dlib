
#pragma once

class CSimpleProp : public CMFCPropertyGridProperty
{
public:
	CSimpleProp(const CString& strGroupName, DWORD_PTR dwData = 0, BOOL bIsValueList = FALSE)
		: CMFCPropertyGridProperty(strGroupName, dwData, bIsValueList)
	{
	}

	CSimpleProp(const CString& strName, const COleVariant& varValue, LPCTSTR lpszDescr = NULL, DWORD_PTR dwData = 0,
		LPCTSTR lpszEditMask = NULL, LPCTSTR lpszEditTemplate = NULL, LPCTSTR lpszValidChars = NULL)
		: CMFCPropertyGridProperty(strName, varValue, lpszDescr, dwData, lpszEditMask, lpszEditTemplate, lpszValidChars)
	{
	}

	virtual void SetValue(const COleVariant& varValue);
};

class CColorProp : public CMFCPropertyGridColorProperty
{
public:
	CColorProp(const CString& strName, const COLORREF& color, CPalette* pPalette = NULL, LPCTSTR lpszDescr = NULL, DWORD_PTR dwData = 0)
		: CMFCPropertyGridColorProperty(strName, color, pPalette, lpszDescr, dwData)
	{
	}

	void SetColor(COLORREF color);
};

class CFileProp : public CMFCPropertyGridFileProperty
{
public:
	CFileProp(const CString& strName, BOOL bOpenFileDialog, const CString& strFileName, LPCTSTR lpszDefExt = NULL,
		DWORD dwFlags = OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, LPCTSTR lpszFilter = NULL, LPCTSTR lpszDescr = NULL, DWORD_PTR dwData = 0)
		: CMFCPropertyGridFileProperty(strName, bOpenFileDialog, strFileName, lpszDefExt, dwFlags, lpszFilter, lpszDescr, dwData)
	{
	}

	virtual void OnClickButton(CPoint point);

	virtual void SetValue(const COleVariant& varValue);
};

class CSliderProp : public CSimpleProp
{
public:
	CSliderProp(const CString& strName, const COleVariant& varValue, LPCTSTR lpszDescr = NULL, DWORD_PTR dwData = 0)
		: CSimpleProp(strName, varValue, lpszDescr, dwData)
	{
	}

	virtual BOOL OnUpdateValue();

	virtual CWnd* CreateInPlaceEdit(CRect rectEdit, BOOL& bDefaultFormat);

	virtual BOOL OnSetCursor() const { return FALSE; }
};

class CPropSliderCtrl : public CSliderCtrl
{
public:
	CPropSliderCtrl(CSliderProp* pProp)
		: m_pProp(pProp)
	{
	}

	CSliderProp* m_pProp;

	DECLARE_MESSAGE_MAP()

	afx_msg void HScroll(UINT nSBCode, UINT nPos);

	afx_msg void OnKillFocus(CWnd* pNewWnd);
};

class CComboProp : public CSimpleProp
{
public:
	CComboProp(const CString& strName, const COleVariant& varValue, LPCTSTR lpszDescr = NULL, DWORD_PTR dwData = 0)
		: CSimpleProp(strName, varValue, lpszDescr, dwData)
		, m_iSelIndex(-1)
	{
	}

	virtual void OnSelectCombo();

	int m_iSelIndex;
};

class CCheckBoxProp : public CSimpleProp
{
public:
	CCheckBoxProp(const CString& strName, BOOL bCheck, LPCTSTR lpszDescr = NULL, DWORD dwData = 0)
		: CSimpleProp(strName, COleVariant((long)bCheck), lpszDescr, dwData)
	{
		m_rectCheck.SetRectEmpty();
	}

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
	CPropertiesWnd()
		: m_bIsPropInvalid(FALSE)
		, m_hSelectedNode(NULL)
	{
	}

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
		PropertyItemLocation,
		PropertyItemLocationX,
		PropertyItemLocationY,
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
		PropertyItemTextAlign,
		PropertyItemTextWrap,
		PropertyItemTextOff,
		PropertyItemTextOffX,
		PropertyItemTextOffY,
		PropertyCount
	};

	CMFCPropertyGridProperty * m_pProp[PropertyCount];

	BOOL m_bIsPropInvalid;

	HTREEITEM m_hSelectedNode;

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

