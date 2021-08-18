#pragma once
#include <afxdockablepane.h>
#include <boost/multi_index_container.hpp>
#include <boost/multi_index/random_access_index.hpp>
#include <boost/multi_index/ordered_index.hpp>
#include <boost/multi_index/hashed_index.hpp>
#include <boost/multi_index/member.hpp>
class COutlinerWnd :
    public CDockablePane
{
public:
    COutlinerWnd() noexcept;

    virtual ~COutlinerWnd();

    struct ListItem
    {
        my::NamedObject* obj;
        std::basic_string<TCHAR> name;
        ListItem(my::NamedObject* _obj, const std::basic_string<TCHAR>& _name)
            : obj(_obj)
            , name(_name)
        {
        }
    };

    typedef boost::multi_index_container<ListItem,
        boost::multi_index::indexed_by<
        boost::multi_index::random_access<>,
        boost::multi_index::hashed_unique<BOOST_MULTI_INDEX_MEMBER(ListItem, std::basic_string<TCHAR>, name) > > > ListItemSet;

    ListItemSet m_Items;

    BOOL m_IgnoreNamedObjectRemoved;

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
    void OnSelectionChanged(my::EventArg* arg);
    afx_msg void OnLvnGetdispinfoList(NMHDR* pNMHDR, LRESULT* pResult);
    afx_msg void OnLvnOdcachehintList(NMHDR* pNMHDR, LRESULT* pResult);
    afx_msg void OnLvnOdfinditemList(NMHDR* pNMHDR, LRESULT* pResult);
};

