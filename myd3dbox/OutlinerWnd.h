#pragma once
#include <afxdockablepane.h>
class COutlinerWnd :
    public CDockablePane
{
public:
    COutlinerWnd() noexcept;

    virtual ~COutlinerWnd();

protected:
    CListCtrl m_listCtrl;

protected:
    afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
    afx_msg void OnSize(UINT nType, int cx, int cy);

    DECLARE_MESSAGE_MAP()
public:
    afx_msg void OnDestroy();
    void OnNamedObjectAdded(my::EventArg* arg);
    void OnNamedObjectRemoved(my::EventArg* arg);
};

