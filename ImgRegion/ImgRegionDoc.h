#pragma once

#include <afxwin.h>

class CImgRegionDoc : public CDocument
{
	DECLARE_DYNCREATE(CImgRegionDoc)

public:
	CImgRegionDoc(void);

	virtual BOOL OnNewDocument();

	DECLARE_MESSAGE_MAP()
};
