#include <windows.h>
#include "PrintCallStack.h"
#include "myDxutApp11.h"

using namespace my;

LONG WINAPI OnException(_EXCEPTION_POINTERS* ExceptionInfo)
{
	WriteMiniDump(ExceptionInfo, _T("aaa.dmp"));
	return EXCEPTION_EXECUTE_HANDLER;
}

INT WINAPI wWinMain( HINSTANCE, HINSTANCE, LPWSTR, int )
{
    // Enable run-time memory check for debug builds.
#if defined(DEBUG) | defined(_DEBUG)
    _CrtSetDbgFlag( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
#endif

	SetUnhandledExceptionFilter(OnException);

	DxutApp11 app;
	return app.Run();
}
