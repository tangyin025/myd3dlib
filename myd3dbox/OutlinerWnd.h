#pragma once
#include <afxdockablepane.h>
#include <boost/multi_index_container.hpp>
#include <boost/multi_index/random_access_index.hpp>
#include <boost/multi_index/hashed_index.hpp>
#include <boost/multi_index/identity.hpp>
class COutlinerWnd :
    public CDockablePane
{
public:
    COutlinerWnd() noexcept;

    virtual ~COutlinerWnd();

    typedef boost::multi_index_container<my::NamedObject *, boost::multi_index::indexed_by<
        boost::multi_index::random_access<>,
        boost::multi_index::hashed_unique<boost::multi_index::identity<my::NamedObject *> > > > ItemSet;

    ItemSet m_Items;

    BOOL m_IgnoreNamedObjectRemoved;

protected:
    CListCtrl m_listCtrl;

protected:
    afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
    afx_msg void OnSize(UINT nType, int cx, int cy);

    DECLARE_MESSAGE_MAP()
public:
    afx_msg void OnDestroy();
    void OnInitItemList();
    void OnDestroyItemList();
    void OnNamedObjectAdded(my::EventArg* arg);
    void OnNamedObjectRemoved(my::EventArg* arg);
    void OnSelectionChanged(my::EventArg* arg);
    void OnAttributeChanged(my::EventArg* arg);
    afx_msg void OnLvnGetdispinfoList(NMHDR* pNMHDR, LRESULT* pResult);
    afx_msg void OnLvnOdcachehintList(NMHDR* pNMHDR, LRESULT* pResult);
    afx_msg void OnLvnOdfinditemList(NMHDR* pNMHDR, LRESULT* pResult);
    afx_msg void OnNotifyClick(NMHDR* pNMHDR, LRESULT* pResult);
};

