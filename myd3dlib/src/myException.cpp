
#include "stdafx.h"
#include "myException.h"
#include <sstream>
#include <d3d9.h>

namespace my
{
	Exception::Exception(const std::basic_string<_TCHAR> & file, int line)
		: m_file(file)
		, m_line(line)
	{
		_ASSERT(false);
	}

	Exception::~Exception(void)
	{
	}

	std::basic_string<_TCHAR> Exception::GetFullDescription(void) const
	{
		std::basic_stringstream<_TCHAR> osstr;
		osstr << m_file << _T(" (") << m_line << _T("):") << std::endl;
		osstr << GetDescription();
		return osstr.str();
	}

	ComException::ComException(HRESULT hres, const std::basic_string<_TCHAR> & file, int line)
		: Exception(file, line)
		, m_hres(hres)
	{
	}

	std::basic_string<_TCHAR> ComException::GetDescription(void) const throw()
	{
		switch(m_hres)
		{
		case NOERROR: return _T("NOERROR");
		case E_UNEXPECTED: return _T("E_UNEXPECTED");
#if defined(_WIN32) && !defined(_MAC)
		case E_NOTIMPL: return _T("E_NOTIMPL");
		case E_OUTOFMEMORY: return _T("E_OUTOFMEMORY");
		case E_INVALIDARG: return _T("E_INVALIDARG");
		case E_NOINTERFACE: return _T("E_NOINTERFACE");
		case E_POINTER: return _T("E_POINTER");
		case E_HANDLE: return _T("E_HANDLE");
		case E_ABORT: return _T("E_ABORT");
		case E_FAIL: return _T("E_FAIL");
		case E_ACCESSDENIED: return _T("E_ACCESSDENIED");
#else
		case E_NOTIMPL: return _T("E_NOTIMPL");
		case E_OUTOFMEMORY: return _T("E_OUTOFMEMORY");
		case E_INVALIDARG: return _T("E_INVALIDARG");
		case E_NOINTERFACE: return _T("E_NOINTERFACE");
		case E_POINTER: return _T("E_POINTER");
		case E_HANDLE: return _T("E_HANDLE");
		case E_ABORT: return _T("E_ABORT");
		case E_FAIL: return _T("E_FAIL");
		case E_ACCESSDENIED: return _T("E_ACCESSDENIED");
#endif //WIN32
		case E_PENDING: return _T("E_PENDING");
		case CO_E_INIT_TLS: return _T("CO_E_INIT_TLS");
		case CO_E_INIT_SHARED_ALLOCATOR: return _T("CO_E_INIT_SHARED_ALLOCATOR");
		case CO_E_INIT_MEMORY_ALLOCATOR: return _T("CO_E_INIT_MEMORY_ALLOCATOR");
		case CO_E_INIT_CLASS_CACHE: return _T("CO_E_INIT_CLASS_CACHE");
		case CO_E_INIT_RPC_CHANNEL: return _T("CO_E_INIT_RPC_CHANNEL");
		case CO_E_INIT_TLS_SET_CHANNEL_CONTROL: return _T("CO_E_INIT_TLS_SET_CHANNEL_CONTROL");
		case CO_E_INIT_TLS_CHANNEL_CONTROL: return _T("CO_E_INIT_TLS_CHANNEL_CONTROL");
		case CO_E_INIT_UNACCEPTED_USER_ALLOCATOR: return _T("CO_E_INIT_UNACCEPTED_USER_ALLOCATOR");
		case CO_E_INIT_SCM_MUTEX_EXISTS: return _T("CO_E_INIT_SCM_MUTEX_EXISTS");
		case CO_E_INIT_SCM_FILE_MAPPING_EXISTS: return _T("CO_E_INIT_SCM_FILE_MAPPING_EXISTS");
		case CO_E_INIT_SCM_MAP_VIEW_OF_FILE: return _T("CO_E_INIT_SCM_MAP_VIEW_OF_FILE");
		case CO_E_INIT_SCM_EXEC_FAILURE: return _T("CO_E_INIT_SCM_EXEC_FAILURE");
		case CO_E_INIT_ONLY_SINGLE_THREADED: return _T("CO_E_INIT_ONLY_SINGLE_THREADED");
		case CO_E_CANT_REMOTE: return _T("CO_E_CANT_REMOTE");
		case CO_E_BAD_SERVER_NAME: return _T("CO_E_BAD_SERVER_NAME");
		case CO_E_WRONG_SERVER_IDENTITY: return _T("CO_E_WRONG_SERVER_IDENTITY");
		case CO_E_OLE1DDE_DISABLED: return _T("CO_E_OLE1DDE_DISABLED");
		case CO_E_RUNAS_SYNTAX: return _T("CO_E_RUNAS_SYNTAX");
		case CO_E_CREATEPROCESS_FAILURE: return _T("CO_E_CREATEPROCESS_FAILURE");
		case CO_E_RUNAS_CREATEPROCESS_FAILURE: return _T("CO_E_RUNAS_CREATEPROCESS_FAILURE");
		case CO_E_RUNAS_LOGON_FAILURE: return _T("CO_E_RUNAS_LOGON_FAILURE");
		case CO_E_LAUNCH_PERMSSION_DENIED: return _T("CO_E_LAUNCH_PERMSSION_DENIED");
		case CO_E_START_SERVICE_FAILURE: return _T("CO_E_START_SERVICE_FAILURE");
		case CO_E_REMOTE_COMMUNICATION_FAILURE: return _T("CO_E_REMOTE_COMMUNICATION_FAILURE");
		case CO_E_SERVER_START_TIMEOUT: return _T("CO_E_SERVER_START_TIMEOUT");
		case CO_E_CLSREG_INCONSISTENT: return _T("CO_E_CLSREG_INCONSISTENT");
		case CO_E_IIDREG_INCONSISTENT: return _T("CO_E_IIDREG_INCONSISTENT");
		case CO_E_NOT_SUPPORTED: return _T("CO_E_NOT_SUPPORTED");
		case CO_E_RELOAD_DLL: return _T("CO_E_RELOAD_DLL");
		case CO_E_MSI_ERROR: return _T("CO_E_MSI_ERROR");
		case CO_E_ATTEMPT_TO_CREATE_OUTSIDE_CLIENT_CONTEXT: return _T("CO_E_ATTEMPT_TO_CREATE_OUTSIDE_CLIENT_CONTEXT");
		case CO_E_SERVER_PAUSED: return _T("CO_E_SERVER_PAUSED");
		case CO_E_SERVER_NOT_PAUSED: return _T("CO_E_SERVER_NOT_PAUSED");
		case CO_E_CLASS_DISABLED: return _T("CO_E_CLASS_DISABLED");
		case CO_E_CLRNOTAVAILABLE: return _T("CO_E_CLRNOTAVAILABLE");
		case CO_E_ASYNC_WORK_REJECTED: return _T("CO_E_ASYNC_WORK_REJECTED");
		case CO_E_SERVER_INIT_TIMEOUT: return _T("CO_E_SERVER_INIT_TIMEOUT");
		case CO_E_NO_SECCTX_IN_ACTIVATE: return _T("CO_E_NO_SECCTX_IN_ACTIVATE");
		case CO_E_TRACKER_CONFIG: return _T("CO_E_TRACKER_CONFIG");
		case CO_E_THREADPOOL_CONFIG: return _T("CO_E_THREADPOOL_CONFIG");
		case CO_E_SXS_CONFIG: return _T("CO_E_SXS_CONFIG");
		case CO_E_MALFORMED_SPN: return _T("CO_E_MALFORMED_SPN");
		//case S_OK: return _T("S_OK");
		case S_FALSE: return _T("S_FALSE");
		}
		return _T("unknown error result");
	}

	D3DException::D3DException(HRESULT hres, const std::basic_string<_TCHAR> & file, int line)
		: ComException(hres, file, line)
	{
	}

	std::basic_string<_TCHAR> D3DException::GetDescription(void) const throw()
	{
		switch(m_hres)
		{
		case D3DOK_NOAUTOGEN: return _T("D3DOK_NOAUTOGEN");
		case D3DERR_CONFLICTINGRENDERSTATE: return _T("D3DERR_CONFLICTINGRENDERSTATE");
		case D3DERR_CONFLICTINGTEXTUREFILTER: return _T("D3DERR_CONFLICTINGTEXTUREFILTER");
		case D3DERR_CONFLICTINGTEXTUREPALETTE: return _T("D3DERR_CONFLICTINGTEXTUREPALETTE");
		case D3DERR_DEVICEHUNG: return _T("D3DERR_DEVICEHUNG");
		case D3DERR_DEVICELOST: return _T("D3DERR_DEVICELOST");
		case D3DERR_DEVICENOTRESET: return _T("D3DERR_DEVICENOTRESET");
		case D3DERR_DEVICEREMOVED: return _T("D3DERR_DEVICEREMOVED");
		case D3DERR_DRIVERINTERNALERROR: return _T("D3DERR_DRIVERINTERNALERROR");
		case D3DERR_DRIVERINVALIDCALL: return _T("D3DERR_DRIVERINVALIDCALL");
		case D3DERR_INVALIDCALL: return _T("D3DERR_INVALIDCALL");
		case D3DERR_INVALIDDEVICE: return _T("D3DERR_INVALIDDEVICE");
		case D3DERR_MOREDATA: return _T("D3DERR_MOREDATA");
		case D3DERR_NOTAVAILABLE: return _T("D3DERR_NOTAVAILABLE");
		case D3DERR_NOTFOUND: return _T("D3DERR_NOTFOUND");
		case D3D_OK: return _T("D3D_OK");
		case D3DERR_OUTOFVIDEOMEMORY: return _T("D3DERR_OUTOFVIDEOMEMORY");
		case D3DERR_TOOMANYOPERATIONS: return _T("D3DERR_TOOMANYOPERATIONS");
		case D3DERR_UNSUPPORTEDALPHAARG: return _T("D3DERR_UNSUPPORTEDALPHAARG");
		case D3DERR_UNSUPPORTEDALPHAOPERATION: return _T("D3DERR_UNSUPPORTEDALPHAOPERATION");
		case D3DERR_UNSUPPORTEDCOLORARG: return _T("D3DERR_UNSUPPORTEDCOLORARG");
		case D3DERR_UNSUPPORTEDCOLOROPERATION: return _T("D3DERR_UNSUPPORTEDCOLOROPERATION");
		case D3DERR_UNSUPPORTEDFACTORVALUE: return _T("D3DERR_UNSUPPORTEDFACTORVALUE");
		case D3DERR_UNSUPPORTEDTEXTUREFILTER: return _T("D3DERR_UNSUPPORTEDTEXTUREFILTER");
		case D3DERR_WASSTILLDRAWING: return _T("D3DERR_WASSTILLDRAWING");
		case D3DERR_WRONGTEXTUREFORMAT: return _T("D3DERR_WRONGTEXTUREFORMAT");
		case E_FAIL: return _T("E_FAIL");
		case E_INVALIDARG: return _T("E_INVALIDARG");
		//case E_INVALIDCALL: return _T("E_INVALIDCALL");
		case E_NOINTERFACE: return _T("E_NOINTERFACE");
		case E_NOTIMPL: return _T("E_NOTIMPL");
		case E_OUTOFMEMORY: return _T("E_OUTOFMEMORY");
		//case S_OK: return _T("S_OK");
		}
		return ComException::GetDescription();
	}

	WinException::WinException(DWORD code, const std::basic_string<_TCHAR> & file, int line)
		: Exception(file, line)
		, m_code(code)
	{
	}

	std::basic_string<_TCHAR> WinException::GetDescription(void) const throw()
	{
		std::basic_string<_TCHAR> desc;
		desc.resize(MAX_PATH);
		desc.resize(::FormatMessage(
			FORMAT_MESSAGE_FROM_SYSTEM, NULL, m_code, MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US), &desc[0], desc.size(), NULL));

		if(desc.empty())
		{
			return _T("unknown windows error");
		}

		return desc;
	}

	CustomException::CustomException(const std::basic_string<_TCHAR> & desc, const std::basic_string<_TCHAR> & file, int line)
		: Exception(file, line)
		, m_desc(desc)
	{
	}

	std::basic_string<_TCHAR> CustomException::GetDescription(void) const throw()
	{
		return m_desc;
	}
};
