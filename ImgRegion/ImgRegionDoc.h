#pragma once

class CImgRegionDoc : public CDocument
{
	DECLARE_DYNCREATE(CImgRegionDoc)

public:
	CImgRegionDoc(void);

	virtual BOOL OnNewDocument(void);

	DECLARE_MESSAGE_MAP()
};
