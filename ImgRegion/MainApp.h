#pragma once

#include "resource.h"

typedef boost::shared_ptr<Gdiplus::Image> ImagePtr;

typedef boost::shared_ptr<Gdiplus::Font> FontPtr2;

class CMainApp : public CWinAppEx
{
public:
	CMainApp(void);

	virtual BOOL InitInstance(void);

	UINT  m_nAppLook;

	Gdiplus::GdiplusStartupInput gdiplusStartupInput;

	ULONG_PTR gdiplusToken;

	CArray<Gdiplus::FontFamily> fontFamilies;

	typedef std::tr1::unordered_map<std::wstring, CString, boost::hash<std::wstring> > ImageMIMEMap;

	ImageMIMEMap m_ImageMIME;

	const CString & GetMIME(const CString & strExt);

	typedef std::tr1::unordered_map<std::wstring, ImagePtr, boost::hash<std::wstring> > ImagePtrMap;

	ImagePtrMap m_ImageMap;

	ImagePtr GetImage(const CString & strImg);

	typedef std::tr1::unordered_map<std::wstring, FontPtr2, boost::hash<std::wstring> > FontPtr2Map;

	FontPtr2Map m_FontMap;

	FontPtr2 GetFont(const CString & strFamily, float fSize);

	DECLARE_MESSAGE_MAP()

	virtual int ExitInstance();

	virtual BOOL OnIdle(LONG lCount);

	afx_msg void OnAppAbout();
};

extern CMainApp theApp;
