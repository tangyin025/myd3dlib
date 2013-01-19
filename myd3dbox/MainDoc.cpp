#include "StdAfx.h"
#include "MainDoc.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

CMainDoc::SingleInstance * my::SingleInstance<CMainDoc>::s_ptr(NULL);

IMPLEMENT_DYNCREATE(CMainDoc, CDocument)

CMainDoc::CMainDoc(void)
{
}
