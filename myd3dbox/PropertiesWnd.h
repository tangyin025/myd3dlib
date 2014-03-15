
#pragma once

typedef boost::function<void (void)> PropertyEvent;

class CSimpleProp : public CMFCPropertyGridProperty
{
public:
	DECLARE_DYNAMIC(CSimpleProp)

	PropertyEvent m_EventChanged;

	PropertyEvent m_EventUpdated;

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

	void OnEventChanged(void);

	void OnEventUpdated(void);

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

class CPropertiesToolBar : public CMFCToolBar
{
public:
	virtual void OnUpdateCmdUI(CFrameWnd* /*pTarget*/, BOOL bDisableIfNoHndler)
	{
		CMFCToolBar::OnUpdateCmdUI((CFrameWnd*) GetOwner(), bDisableIfNoHndler);
	}

	virtual BOOL AllowShowOnList() const { return FALSE; }
};

class TreeNodeBase;

class CPropertiesWnd
	: public CDockablePane
	, public my::SingleInstance<CPropertiesWnd>
{
public:
	CPropertiesWnd()
		: m_bIsPropInvalid(TRUE)
	{
	}

	DECLARE_MESSAGE_MAP()
public:
	CPropertiesToolBar m_wndToolBar;

	CMFCPropertyGridCtrl m_wndPropList;

	boost::weak_ptr<TreeNodeBase> m_SelectedNode;

	BOOL m_bIsPropInvalid;

	void AdjustLayout();

	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);

	afx_msg void OnSize(UINT nType, int cx, int cy);

	afx_msg void OnSetFocus(CWnd* pOldWnd);

	afx_msg void OnExpandAllProperties();

	afx_msg void OnUpdateExpandAllProperties(CCmdUI* pCmdUI);

	afx_msg void OnSortProperties();

	afx_msg void OnUpdateSortProperties(CCmdUI* pCmdUI);

	afx_msg LRESULT OnPropertyChanged(WPARAM wParam, LPARAM lParam);

	afx_msg LRESULT OnIdleUpdateCmdUI(WPARAM wParam, LPARAM lParam);
};
