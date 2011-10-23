
#include "stdafx.h"
#include "myd3dlib.h"

namespace my
{
	Exception::Exception(const std::string & file, int line)
		: m_file(file)
		, m_line(line)
	{
		_ASSERT(false);
	}

	Exception::~Exception(void)
	{
	}

	std::string Exception::GetFullDescription(void) const
	{
		std::stringstream osstr;
		osstr << m_file << " (" << m_line << "):" << std::endl;
		osstr << GetDescription();
		return osstr.str();
	}

	ComException::ComException(HRESULT hres, const std::string & file, int line)
		: Exception(file, line)
		, m_hres(hres)
	{
	}

	std::string ComException::GetDescription(void) const throw()
	{
		switch(m_hres)
		{
		case NOERROR: return "NOERROR";
		case E_UNEXPECTED: return "E_UNEXPECTED";
#if defined(_WIN32) && !defined(_MAC)
		case E_NOTIMPL: return "E_NOTIMPL";
		case E_OUTOFMEMORY: return "E_OUTOFMEMORY";
		case E_INVALIDARG: return "E_INVALIDARG";
		case E_NOINTERFACE: return "E_NOINTERFACE";
		case E_POINTER: return "E_POINTER";
		case E_HANDLE: return "E_HANDLE";
		case E_ABORT: return "E_ABORT";
		case E_FAIL: return "E_FAIL";
		case E_ACCESSDENIED: return "E_ACCESSDENIED";
#else
		case E_NOTIMPL: return "E_NOTIMPL";
		case E_OUTOFMEMORY: return "E_OUTOFMEMORY";
		case E_INVALIDARG: return "E_INVALIDARG";
		case E_NOINTERFACE: return "E_NOINTERFACE";
		case E_POINTER: return "E_POINTER";
		case E_HANDLE: return "E_HANDLE";
		case E_ABORT: return "E_ABORT";
		case E_FAIL: return "E_FAIL";
		case E_ACCESSDENIED: return "E_ACCESSDENIED";
#endif //WIN32
		case E_PENDING: return "E_PENDING";
		case CO_E_INIT_TLS: return "CO_E_INIT_TLS";
		case CO_E_INIT_SHARED_ALLOCATOR: return "CO_E_INIT_SHARED_ALLOCATOR";
		case CO_E_INIT_MEMORY_ALLOCATOR: return "CO_E_INIT_MEMORY_ALLOCATOR";
		case CO_E_INIT_CLASS_CACHE: return "CO_E_INIT_CLASS_CACHE";
		case CO_E_INIT_RPC_CHANNEL: return "CO_E_INIT_RPC_CHANNEL";
		case CO_E_INIT_TLS_SET_CHANNEL_CONTROL: return "CO_E_INIT_TLS_SET_CHANNEL_CONTROL";
		case CO_E_INIT_TLS_CHANNEL_CONTROL: return "CO_E_INIT_TLS_CHANNEL_CONTROL";
		case CO_E_INIT_UNACCEPTED_USER_ALLOCATOR: return "CO_E_INIT_UNACCEPTED_USER_ALLOCATOR";
		case CO_E_INIT_SCM_MUTEX_EXISTS: return "CO_E_INIT_SCM_MUTEX_EXISTS";
		case CO_E_INIT_SCM_FILE_MAPPING_EXISTS: return "CO_E_INIT_SCM_FILE_MAPPING_EXISTS";
		case CO_E_INIT_SCM_MAP_VIEW_OF_FILE: return "CO_E_INIT_SCM_MAP_VIEW_OF_FILE";
		case CO_E_INIT_SCM_EXEC_FAILURE: return "CO_E_INIT_SCM_EXEC_FAILURE";
		case CO_E_INIT_ONLY_SINGLE_THREADED: return "CO_E_INIT_ONLY_SINGLE_THREADED";
		case CO_E_CANT_REMOTE: return "CO_E_CANT_REMOTE";
		case CO_E_BAD_SERVER_NAME: return "CO_E_BAD_SERVER_NAME";
		case CO_E_WRONG_SERVER_IDENTITY: return "CO_E_WRONG_SERVER_IDENTITY";
		case CO_E_OLE1DDE_DISABLED: return "CO_E_OLE1DDE_DISABLED";
		case CO_E_RUNAS_SYNTAX: return "CO_E_RUNAS_SYNTAX";
		case CO_E_CREATEPROCESS_FAILURE: return "CO_E_CREATEPROCESS_FAILURE";
		case CO_E_RUNAS_CREATEPROCESS_FAILURE: return "CO_E_RUNAS_CREATEPROCESS_FAILURE";
		case CO_E_RUNAS_LOGON_FAILURE: return "CO_E_RUNAS_LOGON_FAILURE";
		case CO_E_LAUNCH_PERMSSION_DENIED: return "CO_E_LAUNCH_PERMSSION_DENIED";
		case CO_E_START_SERVICE_FAILURE: return "CO_E_START_SERVICE_FAILURE";
		case CO_E_REMOTE_COMMUNICATION_FAILURE: return "CO_E_REMOTE_COMMUNICATION_FAILURE";
		case CO_E_SERVER_START_TIMEOUT: return "CO_E_SERVER_START_TIMEOUT";
		case CO_E_CLSREG_INCONSISTENT: return "CO_E_CLSREG_INCONSISTENT";
		case CO_E_IIDREG_INCONSISTENT: return "CO_E_IIDREG_INCONSISTENT";
		case CO_E_NOT_SUPPORTED: return "CO_E_NOT_SUPPORTED";
		case CO_E_RELOAD_DLL: return "CO_E_RELOAD_DLL";
		case CO_E_MSI_ERROR: return "CO_E_MSI_ERROR";
		case CO_E_ATTEMPT_TO_CREATE_OUTSIDE_CLIENT_CONTEXT: return "CO_E_ATTEMPT_TO_CREATE_OUTSIDE_CLIENT_CONTEXT";
		case CO_E_SERVER_PAUSED: return "CO_E_SERVER_PAUSED";
		case CO_E_SERVER_NOT_PAUSED: return "CO_E_SERVER_NOT_PAUSED";
		case CO_E_CLASS_DISABLED: return "CO_E_CLASS_DISABLED";
		case CO_E_CLRNOTAVAILABLE: return "CO_E_CLRNOTAVAILABLE";
		case CO_E_ASYNC_WORK_REJECTED: return "CO_E_ASYNC_WORK_REJECTED";
		case CO_E_SERVER_INIT_TIMEOUT: return "CO_E_SERVER_INIT_TIMEOUT";
		case CO_E_NO_SECCTX_IN_ACTIVATE: return "CO_E_NO_SECCTX_IN_ACTIVATE";
		case CO_E_TRACKER_CONFIG: return "CO_E_TRACKER_CONFIG";
		case CO_E_THREADPOOL_CONFIG: return "CO_E_THREADPOOL_CONFIG";
		case CO_E_SXS_CONFIG: return "CO_E_SXS_CONFIG";
		case CO_E_MALFORMED_SPN: return "CO_E_MALFORMED_SPN";
		//case S_OK: return "S_OK";
		case S_FALSE: return "S_FALSE";
		}
		return "unknown error result";
	}

	D3DException::D3DException(HRESULT hres, const std::string & file, int line)
		: ComException(hres, file, line)
	{
	}

	std::string D3DException::GetDescription(void) const throw()
	{
		switch(m_hres)
		{
		case D3DOK_NOAUTOGEN: return "D3DOK_NOAUTOGEN";
		case D3DERR_CONFLICTINGRENDERSTATE: return "D3DERR_CONFLICTINGRENDERSTATE";
		case D3DERR_CONFLICTINGTEXTUREFILTER: return "D3DERR_CONFLICTINGTEXTUREFILTER";
		case D3DERR_CONFLICTINGTEXTUREPALETTE: return "D3DERR_CONFLICTINGTEXTUREPALETTE";
		case D3DERR_DEVICEHUNG: return "D3DERR_DEVICEHUNG";
		case D3DERR_DEVICELOST: return "D3DERR_DEVICELOST";
		case D3DERR_DEVICENOTRESET: return "D3DERR_DEVICENOTRESET";
		case D3DERR_DEVICEREMOVED: return "D3DERR_DEVICEREMOVED";
		case D3DERR_DRIVERINTERNALERROR: return "D3DERR_DRIVERINTERNALERROR";
		case D3DERR_DRIVERINVALIDCALL: return "D3DERR_DRIVERINVALIDCALL";
		case D3DERR_INVALIDCALL: return "D3DERR_INVALIDCALL";
		case D3DERR_INVALIDDEVICE: return "D3DERR_INVALIDDEVICE";
		case D3DERR_MOREDATA: return "D3DERR_MOREDATA";
		case D3DERR_NOTAVAILABLE: return "D3DERR_NOTAVAILABLE";
		case D3DERR_NOTFOUND: return "D3DERR_NOTFOUND";
		case D3D_OK: return "D3D_OK";
		case D3DERR_OUTOFVIDEOMEMORY: return "D3DERR_OUTOFVIDEOMEMORY";
		case D3DERR_TOOMANYOPERATIONS: return "D3DERR_TOOMANYOPERATIONS";
		case D3DERR_UNSUPPORTEDALPHAARG: return "D3DERR_UNSUPPORTEDALPHAARG";
		case D3DERR_UNSUPPORTEDALPHAOPERATION: return "D3DERR_UNSUPPORTEDALPHAOPERATION";
		case D3DERR_UNSUPPORTEDCOLORARG: return "D3DERR_UNSUPPORTEDCOLORARG";
		case D3DERR_UNSUPPORTEDCOLOROPERATION: return "D3DERR_UNSUPPORTEDCOLOROPERATION";
		case D3DERR_UNSUPPORTEDFACTORVALUE: return "D3DERR_UNSUPPORTEDFACTORVALUE";
		case D3DERR_UNSUPPORTEDTEXTUREFILTER: return "D3DERR_UNSUPPORTEDTEXTUREFILTER";
		case D3DERR_WASSTILLDRAWING: return "D3DERR_WASSTILLDRAWING";
		case D3DERR_WRONGTEXTUREFORMAT: return "D3DERR_WRONGTEXTUREFORMAT";
		case E_FAIL: return "E_FAIL";
		case E_INVALIDARG: return "E_INVALIDARG";
		//case E_INVALIDCALL: return "E_INVALIDCALL";
		case E_NOINTERFACE: return "E_NOINTERFACE";
		case E_NOTIMPL: return "E_NOTIMPL";
		case E_OUTOFMEMORY: return "E_OUTOFMEMORY";
		//case S_OK: return "S_OK";
		}
		return ComException::GetDescription();
	}

	WinException::WinException(DWORD code, const std::string & file, int line)
		: Exception(file, line)
		, m_code(code)
	{
	}

	std::string WinException::GetDescription(void) const throw()
	{
		std::string desc;
		desc.resize(MAX_PATH);
		desc.resize(::FormatMessageA(
			FORMAT_MESSAGE_FROM_SYSTEM, NULL, m_code, MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US), &desc[0], desc.size(), NULL));

		if(desc.empty())
		{
			return "unknown windows error";
		}

		return desc;
	}

	CustomException::CustomException(const std::string & desc, const std::string & file, int line)
		: Exception(file, line)
		, m_desc(desc)
	{
	}

	std::string CustomException::GetDescription(void) const throw()
	{
		return m_desc;
	}
};
