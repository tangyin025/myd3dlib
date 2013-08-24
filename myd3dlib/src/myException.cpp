#include "stdafx.h"
#include "myException.h"
#include <sstream>
#include <d3d9.h>
#include <dinput.h>
#include <dsound.h>
#include <tchar.h>

using namespace my;

std::basic_string<TCHAR> ComException::Translate(HRESULT hres) throw()
{
	switch(hres)
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

std::basic_string<TCHAR> ComException::what(void) const
{
	std::basic_ostringstream<TCHAR> osstr;
	osstr << m_file << _T(" (") << m_line << _T("):") << std::endl;
	osstr << Translate(m_hres);
	return osstr.str();
}

std::basic_string<TCHAR> D3DException::Translate(HRESULT hres) throw()
{
	switch(hres)
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
	return ComException::Translate(hres);
}

std::basic_string<TCHAR> D3DException::what(void) const
{
	std::basic_ostringstream<TCHAR> osstr;
	osstr << m_file << _T(" (") << m_line << _T("):") << std::endl;
	osstr << Translate(m_hres);
	return osstr.str();
}

std::basic_string<TCHAR> DInputException::Translate(HRESULT hres) throw()
{
	switch(hres)
	{
	case DI_BUFFEROVERFLOW: return _T("DI_BUFFEROVERFLOW");
	case DI_DOWNLOADSKIPPED: return _T("DI_DOWNLOADSKIPPED");
	case DI_EFFECTRESTARTED: return _T("DI_EFFECTRESTARTED");
	//case DI_NOEFFECT: return _T("DI_NOEFFECT");
	//case DI_NOTATTACHED: return _T("DI_NOTATTACHED");
	case DI_OK: return _T("DI_OK");
	case DI_POLLEDDEVICE: return _T("DI_POLLEDDEVICE");
	//case DI_PROPNOEFFECT: return _T("DI_PROPNOEFFECT");
	case DI_TRUNCATED: return _T("DI_TRUNCATED");
	case DI_TRUNCATEDANDRESTARTED: return _T("DI_TRUNCATEDANDRESTARTED");
	case DIERR_ACQUIRED: return _T("DIERR_ACQUIRED");
	case DIERR_ALREADYINITIALIZED: return _T("DIERR_ALREADYINITIALIZED");
	case DIERR_BADDRIVERVER: return _T("DIERR_BADDRIVERVER");
	case DIERR_BETADIRECTINPUTVERSION: return _T("DIERR_BETADIRECTINPUTVERSION");
	case DIERR_DEVICEFULL: return _T("DIERR_DEVICEFULL");
	case DIERR_DEVICENOTREG: return _T("DIERR_DEVICENOTREG");
	case DIERR_EFFECTPLAYING: return _T("DIERR_EFFECTPLAYING");
	case DIERR_HASEFFECTS: return _T("DIERR_HASEFFECTS");
	case DIERR_GENERIC: return _T("DIERR_GENERIC");
	case DIERR_HANDLEEXISTS: return _T("DIERR_HANDLEEXISTS");
	case DIERR_INCOMPLETEEFFECT: return _T("DIERR_INCOMPLETEEFFECT");
	case DIERR_INPUTLOST: return _T("DIERR_INPUTLOST");
	case DIERR_INVALIDPARAM: return _T("DIERR_INVALIDPARAM");
	case DIERR_MOREDATA: return _T("DIERR_MOREDATA");
	case DIERR_NOAGGREGATION: return _T("DIERR_NOAGGREGATION");
	case DIERR_NOINTERFACE: return _T("DIERR_NOINTERFACE");
	case DIERR_NOTACQUIRED: return _T("DIERR_NOTACQUIRED");
	case DIERR_NOTBUFFERED: return _T("DIERR_NOTBUFFERED");
	case DIERR_NOTDOWNLOADED: return _T("DIERR_NOTDOWNLOADED");
	case DIERR_NOTEXCLUSIVEACQUIRED: return _T("DIERR_NOTEXCLUSIVEACQUIRED");
	case DIERR_NOTFOUND: return _T("DIERR_NOTFOUND");
	case DIERR_NOTINITIALIZED: return _T("DIERR_NOTINITIALIZED");
	//case DIERR_OBJECTNOTFOUND: return _T("DIERR_OBJECTNOTFOUND");
	case DIERR_OLDDIRECTINPUTVERSION: return _T("DIERR_OLDDIRECTINPUTVERSION");
	//case DIERR_OTHERAPPHASPRIO: return _T("DIERR_OTHERAPPHASPRIO");
	case DIERR_OUTOFMEMORY: return _T("DIERR_OUTOFMEMORY");
	//case DIERR_READONLY: return _T("DIERR_READONLY");
	case DIERR_REPORTFULL: return _T("DIERR_REPORTFULL");
	case DIERR_UNPLUGGED: return _T("DIERR_UNPLUGGED");
	case DIERR_UNSUPPORTED: return _T("DIERR_UNSUPPORTED");
	case E_HANDLE: return _T("E_HANDLE");
	case E_PENDING: return _T("E_PENDING");
	}
	return ComException::Translate(hres);
}

std::basic_string<TCHAR> DInputException::what(void) const
{
	std::basic_ostringstream<TCHAR> osstr;
	osstr << m_file << _T(" (") << m_line << _T("):") << std::endl;
	osstr << Translate(m_hres);
	return osstr.str();
}

std::basic_string<TCHAR> DSoundException::Translate(HRESULT hres) throw()
{
	switch(hres)
	{
	case DS_OK: return _T("DS_OK");
	case DS_NO_VIRTUALIZATION: return _T("DS_NO_VIRTUALIZATION");
	//case DS_INCOMPLETE: return _T("DS_INCOMPLETE");
	case DSERR_ACCESSDENIED: return _T("DSERR_ACCESSDENIED");
	case DSERR_ALLOCATED: return _T("DSERR_ALLOCATED");
	case DSERR_ALREADYINITIALIZED: return _T("DSERR_ALREADYINITIALIZED");
	case DSERR_BADFORMAT: return _T("DSERR_BADFORMAT");
	case DSERR_BADSENDBUFFERGUID: return _T("DSERR_BADSENDBUFFERGUID");
	case DSERR_BUFFERLOST: return _T("DSERR_BUFFERLOST");
	case DSERR_BUFFERTOOSMALL: return _T("DSERR_BUFFERTOOSMALL");
	case DSERR_CONTROLUNAVAIL: return _T("DSERR_CONTROLUNAVAIL");
	case DSERR_DS8_REQUIRED: return _T("DSERR_DS8_REQUIRED");
	case DSERR_FXUNAVAILABLE: return _T("DSERR_FXUNAVAILABLE");
	case DSERR_GENERIC: return _T("DSERR_GENERIC");
	case DSERR_INVALIDCALL: return _T("DSERR_INVALIDCALL");
	case DSERR_INVALIDPARAM: return _T("DSERR_INVALIDPARAM");
	case DSERR_NOAGGREGATION: return _T("DSERR_NOAGGREGATION");
	case DSERR_NODRIVER: return _T("DSERR_NODRIVER");
	case DSERR_NOINTERFACE: return _T("DSERR_NOINTERFACE");
	case DSERR_OBJECTNOTFOUND: return _T("DSERR_OBJECTNOTFOUND");
	case DSERR_OTHERAPPHASPRIO: return _T("DSERR_OTHERAPPHASPRIO");
	case DSERR_OUTOFMEMORY: return _T("DSERR_OUTOFMEMORY");
	case DSERR_PRIOLEVELNEEDED: return _T("DSERR_PRIOLEVELNEEDED");
	case DSERR_SENDLOOP: return _T("DSERR_SENDLOOP");
	case DSERR_UNINITIALIZED: return _T("DSERR_UNINITIALIZED");
	case DSERR_UNSUPPORTED: return _T("DSERR_UNSUPPORTED");
	}
	return ComException::Translate(hres);
}

std::basic_string<TCHAR> DSoundException::what(void) const
{
	std::basic_ostringstream<TCHAR> osstr;
	osstr << m_file << _T(" (") << m_line << _T("):") << std::endl;
	osstr << Translate(m_hres);
	return osstr.str();
}

std::basic_string<TCHAR> WinException::Translate(DWORD code) throw()
{
	std::basic_string<TCHAR> desc;
	desc.resize(MAX_PATH);
	desc.resize(::FormatMessage(
		FORMAT_MESSAGE_FROM_SYSTEM, NULL, code, MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US), &desc[0], desc.size(), NULL));

	if(desc.empty())
	{
		return _T("unknown windows error");
	}

	return desc;
}

std::basic_string<TCHAR> WinException::what(void) const
{
	std::basic_ostringstream<TCHAR> osstr;
	osstr << m_file << _T(" (") << m_line << _T("):") << std::endl;
	osstr << Translate(m_code);
	return osstr.str();
}

std::basic_string<TCHAR> CustomException::what(void) const
{
	std::basic_ostringstream<TCHAR> osstr;
	osstr << m_file << _T(" (") << m_line << _T("):") << std::endl;
	osstr << m_desc;
	return osstr.str();
}
