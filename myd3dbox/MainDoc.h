
// MainDoc.h : interface of the CMainDoc class
//


#pragma once


class CMainDoc : public CDocument
{
protected: // create from serialization only
	CMainDoc();
	DECLARE_DYNCREATE(CMainDoc)

// Attributes
public:

// Operations
public:

// Overrides
public:
	virtual BOOL OnNewDocument();
	virtual void Serialize(CArchive& ar);

// Implementation
public:
	virtual ~CMainDoc();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// Generated message map functions
protected:
	DECLARE_MESSAGE_MAP()
};


