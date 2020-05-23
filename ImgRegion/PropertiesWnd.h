
#pragma once

#include "../myd3dbox/CtrlProps.h"

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
		PropertyItemRect,
		PropertyItemRectL,
		PropertyItemRectT,
		PropertyItemRectW,
		PropertyItemRectH,
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
	afx_msg LRESULT OnIdleUpdateCmdUI(WPARAM wParam, LPARAM lParam);

	void UpdateProperties(void);

	afx_msg LRESULT OnPropertyChanged(WPARAM wParam, LPARAM lParam);

	void InvalidProperties(void)
	{
		m_bIsPropInvalid = TRUE;
	}
};

