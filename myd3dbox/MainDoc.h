#pragma once

class CMainDoc
	: public CDocument
	, public my::SingleInstance<CMainDoc>
{
public:
	CMainDoc(void);

	DECLARE_DYNCREATE(CMainDoc)

	void OnFrameMove(
		double fTime,
		float fElapsedTime);
};
