#pragma once

#include "resource.h"

class CMainApp : public CWinAppEx
{
public:
	CMainApp(void);

	virtual BOOL InitInstance(void);

	UINT  m_nAppLook;

	Gdiplus::GdiplusStartupInput gdiplusStartupInput;

	ULONG_PTR gdiplusToken;

	CArray<Gdiplus::FontFamily> fontFamilies;

	typedef boost::unordered_map<std::wstring, CString> ImageMIMEMap;

	ImageMIMEMap m_ImageMIME;

	const CString & GetMIME(const CString & strExt);

	typedef boost::unordered_map<std::wstring, boost::weak_ptr<Gdiplus::Image> > ImagePtrMap;

	ImagePtrMap m_ImageMap;

	ImagePtr GetImage(const CString & strImg);

	static ImagePtr OpenTgaImage(const CString & strImg);

	typedef boost::unordered_map<std::wstring, boost::weak_ptr<Gdiplus::Font> > FontPtr2Map;

	FontPtr2Map m_FontMap;

	FontPtr2 GetFont(const CString & strFamily, float fSize);

	CMemFile m_ClipboardFile;

	DECLARE_MESSAGE_MAP()

	virtual int ExitInstance();

	virtual BOOL OnIdle(LONG lCount);

	afx_msg void OnAppAbout();
};

extern CMainApp theApp;
