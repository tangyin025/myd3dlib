#include "stdafx.h"
//////////////////////////////////////////////////////////////////////
//
// SocketAPI.cpp
//
//
//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////
// include files
//////////////////////////////////////////////////

//#include "FileAPI.h"
#include "SocketAPI.h"

#if _WINDOWS
#elif __LINUX__
#include <sys/types.h>			// for accept()
#include <sys/socket.h>
#include <sys/time.h>
#include <arpa/inet.h>			// for inet_xxx()
#include <netinet/in.h>
#include <errno.h>				// for errno
#endif


//////////////////////////////////////////////////
// external variable
//////////////////////////////////////////////////
#if __LINUX__
extern INT errno;
#endif

CHAR Error[_ESIZE] ;
//////////////////////////////////////////////////////////////////////
//
// SOCKET SocketAPI::socket_ex ( INT domain , INT type , INT protocol ) 
//
// exception version of socket()
//
// Parameters
//     domain - AF_INET(internet socket), AF_UNIX(internal socket), ...
//	   type  - SOCK_STREAM(TCP), SOCK_DGRAM(UDP), ...
//     protocol - 0
//
// Return 
//     socket descriptor
//
//
//////////////////////////////////////////////////////////////////////
SOCKET SocketAPI::socket_ex ( INT domain , INT type , INT protocol ) 
{
	//__ENTER_FUNCTION_FOXNET

	SOCKET s = ::socket(domain,type,protocol);

	if ( s == INVALID_SOCKET ) 
	{
#if __LINUX__
		switch ( errno ) 
		{
		case EPROTONOSUPPORT :
		case EMFILE : 
		case ENFILE : 
		case EACCES : 
		case ENOBUFS : 
		default : 
			{
				break;
			}
		}//end of switch
#elif _WINDOWS
		INT iErr = WSAGetLastError() ;
		switch ( iErr ) 
		{
		case WSANOTINITIALISED : 
			strncpy_s( Error, "WSANOTINITIALISED", _ESIZE ) ;
			break ;
		case WSAENETDOWN : 
			strncpy_s( Error, "WSAENETDOWN", _ESIZE ) ;
			break ;
		case WSAEAFNOSUPPORT : 
			strncpy_s( Error, "WSAEAFNOSUPPORT", _ESIZE ) ;
			break ;
		case WSAEINPROGRESS : 
			strncpy_s( Error, "WSAEINPROGRESS", _ESIZE ) ;
			break ;
		case WSAEMFILE : 
			strncpy_s( Error, "WSAEMFILE", _ESIZE ) ;
			break ;
		case WSAENOBUFS : 
			strncpy_s( Error, "WSAENOBUFS", _ESIZE ) ;
			break ;
		case WSAEPROTONOSUPPORT : 
			strncpy_s( Error, "WSAEPROTONOSUPPORT", _ESIZE ) ;
			break ;
		case WSAEPROTOTYPE : 
			strncpy_s( Error, "WSAEPROTOTYPE", _ESIZE ) ;
			break ;
		case WSAESOCKTNOSUPPORT : 
			strncpy_s( Error, "WSAESOCKTNOSUPPORT", _ESIZE ) ;
			break ;
		default : 
			{
			strncpy_s( Error, "UNKNOWN", _ESIZE ) ;
			break ;
			};
		};//end of switch
#endif
	}

	return s;
	
	//__LEAVE_FUNCTION_FOXNET

	return INVALID_SOCKET ;
}


//////////////////////////////////////////////////////////////////////
//
// VOID SocketAPI::bind_ex ( SOCKET s , const struct sockaddr * addr , UINT addrlen ) 
//
// exception version of bind()
//
// Parameters
//     s       - socket descriptor 
//     addr    - socket address structure ( normally struct sockaddr_in )
//     addrlen - length of socket address structure
//
// Return
//     none
//
//
//////////////////////////////////////////////////////////////////////
BOOL SocketAPI::bind_ex ( SOCKET s , const struct sockaddr * addr , UINT addrlen ) 
{
	//__ENTER_FUNCTION_FOXNET

	if ( bind ( s , addr , addrlen ) == SOCKET_ERROR ) 
	{
#if __LINUX__
		switch ( errno ) 
		{
		case EADDRINUSE :
		case EINVAL : 
		case EACCES : 
		case ENOTSOCK : 
		case EBADF : 
		case EROFS : 
		case EFAULT : 
		case ENAMETOOLONG : 
		case ENOENT : 
		case ENOMEM : 
		case ENOTDIR : 
		case ELOOP : 
		default :
			{
				break;
			}
		}//end of switch
#elif _WINDOWS
		INT iErr = WSAGetLastError() ;
		switch ( iErr ) 
		{
		case WSANOTINITIALISED : 
			strncpy_s( Error, "WSAESOCKTNOSUPPORT", _ESIZE ) ;
			break ;
		case WSAENETDOWN : 
			strncpy_s( Error, "WSAENETDOWN", _ESIZE ) ;
			break ;
		case WSAEADDRINUSE : 
			strncpy_s( Error, "WSAEADDRINUSE", _ESIZE ) ;
			break ;
		case WSAEADDRNOTAVAIL : 
			strncpy_s( Error, "WSAEADDRNOTAVAIL", _ESIZE ) ;
			break ;
		case WSAEFAULT : 
			strncpy_s( Error, "WSAEFAULT", _ESIZE ) ;
			break ;
		case WSAEINPROGRESS : 
			strncpy_s( Error, "WSAEINPROGRESS", _ESIZE ) ;
			break ;
		case WSAEINVAL : 
			strncpy_s( Error, "WSAEINVAL", _ESIZE ) ;
			break ;
		case WSAENOBUFS : 
			strncpy_s( Error, "WSAENOBUFS", _ESIZE ) ;
			break ;
		case WSAENOTSOCK : 
			strncpy_s( Error, "WSAENOTSOCK", _ESIZE ) ;
			break ;
		default :
			{
			strncpy_s( Error, "UNKNOWN", _ESIZE ) ;
			break ;
			};
		};//end of switch
#endif

		return FALSE ;
	}
	
	return TRUE ;

	//__LEAVE_FUNCTION_FOXNET

	return FALSE ;
}


//////////////////////////////////////////////////////////////////////
//
// VOID SocketAPI::connect_ex ( SOCKET s , const struct sockaddr * addr , UINT addrlen )
//
// exception version of connect() system call
//
// Parameters
//     s       - socket descriptor
//     addr    - socket address structure
//     addrlen - length of socket address structure
//
// Return
//     none
//
//////////////////////////////////////////////////////////////////////
BOOL SocketAPI::connect_ex ( SOCKET s , const struct sockaddr * addr , UINT addrlen )
{
	//__ENTER_FUNCTION_FOXNET

	if ( connect(s,addr,addrlen) == SOCKET_ERROR ) 
	{
#if __LINUX__
		switch ( errno ) {
		case EALREADY : 
		case EINPROGRESS : 
		case ECONNREFUSED : 
		case EISCONN : 
		case ETIMEDOUT : 
		case ENETUNREACH : 
		case EADDRINUSE : 
		case EBADF : 
		case EFAULT : 
		case ENOTSOCK : 
		default :
			{
				break;
			}
		}//end of switch
#elif _WINDOWS
		INT iErr = WSAGetLastError() ;
		switch ( iErr ) 
		{
		case WSANOTINITIALISED : 
			strncpy_s( Error, "WSANOTINITIALISED", _ESIZE ) ;
			break ;
		case WSAENETDOWN : 
			strncpy_s( Error, "WSAENETDOWN", _ESIZE ) ;
			break ;
		case WSAEADDRINUSE : 
			strncpy_s( Error, "WSAEADDRINUSE", _ESIZE ) ;
			break ;
		case WSAEINTR : 
			strncpy_s( Error, "WSAEINTR", _ESIZE ) ;
			break ;
		case WSAEINPROGRESS : 
			strncpy_s( Error, "WSAEINPROGRESS", _ESIZE ) ;
			break ;
		case WSAEALREADY : 
			strncpy_s( Error, "WSAEALREADY", _ESIZE ) ;
			break ;
		case WSAEADDRNOTAVAIL : 
			strncpy_s( Error, "WSAEADDRNOTAVAIL", _ESIZE ) ;
			break ;
		case WSAEAFNOSUPPORT : 
			strncpy_s( Error, "WSAEAFNOSUPPORT", _ESIZE ) ;
			break ;
		case WSAECONNREFUSED : 
			strncpy_s( Error, "WSAECONNREFUSED", _ESIZE ) ;
			break ;
		case WSAEFAULT : 
			strncpy_s( Error, "WSAEFAULT", _ESIZE ) ;
			break ;
		case WSAEINVAL : 
			strncpy_s( Error, "WSAEINVAL", _ESIZE ) ;
			break ;
		case WSAEISCONN : 
			strncpy_s( Error, "WSAEISCONN", _ESIZE ) ;
			break ;
		case WSAENETUNREACH : 
			strncpy_s( Error, "WSAENETUNREACH", _ESIZE ) ;
			break ;
		case WSAENOBUFS : 
			strncpy_s( Error, "WSAENOBUFS", _ESIZE ) ;
			break ;
		case WSAENOTSOCK : 
			strncpy_s( Error, "WSAENOTSOCK", _ESIZE ) ;
			break ;
		case WSAETIMEDOUT : 
			strncpy_s( Error, "WSAETIMEDOUT", _ESIZE ) ;
			break ;
		case WSAEWOULDBLOCK  : 
			strncpy_s( Error, "WSAEWOULDBLOCK", _ESIZE ) ;
			break ;
		default :
			{
			strncpy_s( Error, "UNKNOWN", _ESIZE ) ;
			break ;
			};
		};//end of switch
#endif
		return FALSE ;
	}

	return TRUE ;
	
	//__LEAVE_FUNCTION_FOXNET

	return FALSE ;
}


//////////////////////////////////////////////////////////////////////
//
// VOID SocketAPI::listen_ex ( SOCKET s , UINT backlog )
//
// exception version of listen()
//
// Parameters
//     s       - socket descriptor
//     backlog - waiting queue length
//
// Return
//     none
//
//
//////////////////////////////////////////////////////////////////////
BOOL SocketAPI::listen_ex ( SOCKET s , UINT backlog ) 
{
	//__ENTER_FUNCTION_FOXNET

	if ( listen( s , backlog ) == SOCKET_ERROR ) 
	{
#if __LINUX__
		switch ( errno ) 
		{
		case EBADF : 
		case ENOTSOCK :
		case EOPNOTSUPP :
		default :
			{
				break;
			}
		}//end of switch
#elif _WINDOWS
		INT iErr = WSAGetLastError() ;
		switch ( iErr ) 
		{
		case WSANOTINITIALISED : 
			strncpy_s( Error, "WSANOTINITIALISED", _ESIZE ) ;
			break ;
		case WSAENETDOWN : 
			strncpy_s( Error, "WSAENETDOWN", _ESIZE ) ;
			break ;
		case WSAEADDRINUSE : 
			strncpy_s( Error, "WSAEADDRINUSE", _ESIZE ) ;
			break ;
		case WSAEINPROGRESS : 
			strncpy_s( Error, "WSAEINPROGRESS", _ESIZE ) ;
			break ;
		case WSAEINVAL : 
			strncpy_s( Error, "WSAEINVAL", _ESIZE ) ;
			break ;
		case WSAEISCONN : 
			strncpy_s( Error, "WSAEISCONN", _ESIZE ) ;
			break ;
		case WSAEMFILE : 
			strncpy_s( Error, "WSAEMFILE", _ESIZE ) ;
			break ;
		case WSAENOBUFS : 
			strncpy_s( Error, "WSAENOBUFS", _ESIZE ) ;
			break ;
		case WSAENOTSOCK : 
			strncpy_s( Error, "WSAENOTSOCK", _ESIZE ) ;
			break ;
		case WSAEOPNOTSUPP : 
			strncpy_s( Error, "WSAEOPNOTSUPP", _ESIZE ) ;
			break ;
		default :
			{
			strncpy_s( Error, "UNKNOWN", _ESIZE ) ;
			break ;
			};
		};//end of switch
#endif

		return FALSE ;
	}

	return TRUE ;
	
	//__LEAVE_FUNCTION_FOXNET

	return FALSE ;
}


//////////////////////////////////////////////////////////////////////
//
//SOCKET SocketAPI::accept_ex ( SOCKET s , struct sockaddr * addr , UINT * addrlen ) 
//
// exception version of accept()
//
// Parameters
//     s       - socket descriptor
//     addr    - socket address structure
//     addrlen - length of socket address structure
//
// Return
//     none
//
//
//////////////////////////////////////////////////////////////////////
SOCKET SocketAPI::accept_ex ( SOCKET s , struct sockaddr * addr , UINT * addrlen )
{
	//__ENTER_FUNCTION_FOXNET

#if __LINUX__
	SOCKET client = accept( s , addr , addrlen );
#elif _WINDOWS
	SOCKET client = accept( s , addr , (int*)addrlen );
#endif
	
	if ( client == INVALID_SOCKET ) 
	{
#if __LINUX__
		switch ( errno ) 
		{

		case EWOULDBLOCK : 

		case ECONNRESET :
		case ECONNABORTED :
		case EPROTO :
		case EINTR :
			// from UNIX Network Programming 2nd, 15.6
			// with nonblocking-socket, ignore above errors

		case EBADF : 
		case ENOTSOCK : 
		case EOPNOTSUPP : 
		case EFAULT : 

		default :
			{
				break;
			}
		}//end of switch
#elif _WINDOWS
		INT iErr = WSAGetLastError() ;
		switch ( iErr ) 
		{
		case WSANOTINITIALISED : 
			strncpy_s( Error, "WSANOTINITIALISED", _ESIZE ) ;
			break ;
		case WSAENETDOWN : 
			strncpy_s( Error, "WSAENETDOWN", _ESIZE ) ;
			break ;
		case WSAEFAULT : 
			strncpy_s( Error, "WSAEFAULT", _ESIZE ) ;
			break ;
		case WSAEINTR : 
			strncpy_s( Error, "WSAEINTR", _ESIZE ) ;
			break ;
		case WSAEINPROGRESS : 
			strncpy_s( Error, "WSAEINPROGRESS", _ESIZE ) ;
			break ;
		case WSAEINVAL : 
			strncpy_s( Error, "WSAEINVAL", _ESIZE ) ;
			break ;
		case WSAEMFILE : 
			strncpy_s( Error, "WSAEMFILE", _ESIZE ) ;
			break ;
		case WSAENOBUFS : 
			strncpy_s( Error, "WSAENOBUFS", _ESIZE ) ;
			break ;
		case WSAENOTSOCK : 
			strncpy_s( Error, "WSAENOTSOCK", _ESIZE ) ;
			break ;
		case WSAEOPNOTSUPP : 
			strncpy_s( Error, "WSAEOPNOTSUPP", _ESIZE ) ;
			break ;
		case WSAEWOULDBLOCK : 
			strncpy_s( Error, "WSAEWOULDBLOCK", _ESIZE ) ;
			break ;
		default :
			{
			strncpy_s( Error, "UNKNOWN", _ESIZE ) ;
			break ;
			};
		};//end of switch
#endif
	} else {
	}

	return client;

	//__LEAVE_FUNCTION_FOXNET

	return INVALID_SOCKET ;
}


//////////////////////////////////////////////////////////////////////
//
// VOID SocketAPI::getsockopt_ex ( SOCKET s , INT level , INT optname , VOID * optval , UINT * optlen )
//
// exception version of getsockopt()
//
// Parameters
//     s       - socket descriptor
//     level   - socket option level ( SOL_SOCKET , ... )
//     optname - socket option name ( SO_REUSEADDR , SO_LINGER , ... )
//     optval  - pointer to contain option value
//     optlen  - length of optval
//
// Return
//     none
//
//
//////////////////////////////////////////////////////////////////////
BOOL SocketAPI::getsockopt_ex ( SOCKET s , INT level , INT optname , VOID * optval , UINT * optlen )
{
	//__ENTER_FUNCTION_FOXNET

#if __LINUX__
	if ( getsockopt( s , level , optname , optval , optlen ) == SOCKET_ERROR ) 
	{
		switch ( errno ) 
		{
		case EBADF : 
		case ENOTSOCK : 
		case ENOPROTOOPT : 
		case EFAULT : 
		default :
			{
				break;
			}
		}//end of switch

		return FALSE ;
	}
#elif _WINDOWS
	if ( getsockopt( s , level , optname , (CHAR*)optval , (int*)optlen ) == SOCKET_ERROR ) 
	{
		INT iErr = WSAGetLastError() ;
		switch ( iErr ) 
		{
		case WSANOTINITIALISED : 
			strncpy_s( Error, "WSANOTINITIALISED", _ESIZE ) ;
			break ;
		case WSAENETDOWN : 
			strncpy_s( Error, "WSAENETDOWN", _ESIZE ) ;
			break ;
		case WSAEFAULT : 
			strncpy_s( Error, "WSAEFAULT", _ESIZE ) ;
			break ;
		case WSAEINPROGRESS : 
			strncpy_s( Error, "WSAEINPROGRESS", _ESIZE ) ;
			break ;
		case WSAEINVAL : 
			strncpy_s( Error, "WSAEINVAL", _ESIZE ) ;
			break ;
		case WSAENOPROTOOPT : 
			strncpy_s( Error, "WSAENOPROTOOPT", _ESIZE ) ;
			break ;
		case WSAENOTSOCK : 
			strncpy_s( Error, "WSAENOTSOCK", _ESIZE ) ;
			break ;
		default : 
			{
			strncpy_s( Error, "UNKNOWN", _ESIZE ) ;
			break ;
			};
		};//end of switch

		return FALSE ;
	}
#endif

	return TRUE ;
	
	//__LEAVE_FUNCTION_FOXNET

	return FALSE ;
}

UINT SocketAPI::getsockopt_ex2 ( SOCKET s , INT level , INT optname , VOID * optval , UINT * optlen )
{
	//__ENTER_FUNCTION_FOXNET

#if __LINUX__
	if ( getsockopt( s , level , optname , optval , optlen ) == SOCKET_ERROR ) 
	{
		switch ( errno ) 
		{
		case EBADF : 
			return 1;
		case ENOTSOCK : 
			return 2;
		case ENOPROTOOPT : 
			return 3;
		case EFAULT : 
			return 4;
		default :
			return 5;
		}//end of switch
	}
	return 0;

#elif _WINDOWS
	if ( getsockopt( s , level , optname , (CHAR*)optval , (int*)optlen ) == SOCKET_ERROR ) 
	{
		INT iErr = WSAGetLastError() ;
		switch ( iErr ) 
		{
		case WSANOTINITIALISED:
			strncpy_s( Error, "WSANOTINITIALISED", _ESIZE ) ;
			break ;
		case WSAENETDOWN:
			strncpy_s( Error, "WSAENETDOWN", _ESIZE ) ;
			break ;
		case WSAEFAULT:
			strncpy_s( Error, "WSAEFAULT", _ESIZE ) ;
			break ;
		case WSAEINPROGRESS:
			strncpy_s( Error, "WSAEINPROGRESS", _ESIZE ) ;
			break ;
		case WSAEINVAL:
			strncpy_s( Error, "WSAEINVAL", _ESIZE ) ;
			break ;
		case WSAENOPROTOOPT:
			strncpy_s( Error, "WSAENOPROTOOPT", _ESIZE ) ;
			break ;
		case WSAENOTSOCK:
			strncpy_s( Error, "WSAENOTSOCK", _ESIZE ) ;
			break ;
		default : 
			{
			strncpy_s( Error, "UNKNOWN", _ESIZE ) ;
			break ;
			};
		}
	}
#endif

	return 0;

	//__LEAVE_FUNCTION_FOXNET

	return 5;
}

//////////////////////////////////////////////////////////////////////
//
// VOID SocketAPI::setsockopt_ex ( SOCKET s , INT level , INT optname , const VOID * optval , UINT optlen )
//
// exception version of setsockopt()
//
// Parameters
//     s       - socket descriptor
//     level   - socket option level ( SOL_SOCKET , ... )
//     optname - socket option name ( SO_REUSEADDR , SO_LINGER , ... )
//     optval  - pointer to contain option value
//     optlen  - length of optval
//
// Return
//     none
//
//
//////////////////////////////////////////////////////////////////////
BOOL SocketAPI::setsockopt_ex ( SOCKET s , INT level , INT optname , const VOID * optval , UINT optlen )
{
	//__ENTER_FUNCTION_FOXNET

#if __LINUX__
	if ( setsockopt( s , level , optname , optval , optlen ) == SOCKET_ERROR ) 
	{
		switch ( errno ) 
		{
			case EBADF : 
			case ENOTSOCK : 
			case ENOPROTOOPT : 
			case EFAULT : 
			default :
				{
					break;
				}
		}//end of switch

		return FALSE ;
	}
#elif _WINDOWS
	if ( setsockopt( s , level , optname , (CHAR*)optval , optlen ) == SOCKET_ERROR ) 
	{
		INT iErr = WSAGetLastError() ;
		switch ( iErr ) 
		{
		case WSANOTINITIALISED : 
			strncpy_s( Error, "WSANOTINITIALISED", _ESIZE ) ;
			break ;
		case WSAENETDOWN : 
			strncpy_s( Error, "WSAENETDOWN", _ESIZE ) ;
			break ;
		case WSAEFAULT : 
			strncpy_s( Error, "WSAEFAULT", _ESIZE ) ;
			break ;
		case WSAEINPROGRESS : 
			strncpy_s( Error, "WSAEINPROGRESS", _ESIZE ) ;
			break ;
		case WSAEINVAL : 
			strncpy_s( Error, "WSAEINVAL", _ESIZE ) ;
			break ;
		case WSAENETRESET : 
			strncpy_s( Error, "WSAENETRESET", _ESIZE ) ;
			break ;
		case WSAENOPROTOOPT : 
			strncpy_s( Error, "WSAENOPROTOOPT", _ESIZE ) ;
			break ;
		case WSAENOTCONN : 
			strncpy_s( Error, "WSAENOTCONN", _ESIZE ) ;
			break ;
		case WSAENOTSOCK : 
			strncpy_s( Error, "WSAENOTSOCK", _ESIZE ) ;
			break ;
		default :
			{
			strncpy_s( Error, "UNKNOWN", _ESIZE ) ;
			break ;
			};
		};//end of switch

		return FALSE ;
	}
#endif

	return TRUE ;
	
	//__LEAVE_FUNCTION_FOXNET

	return FALSE ;
}


//////////////////////////////////////////////////////////////////////
//
// UINT SocketAPI::send_ex ( SOCKET s , const VOID * buf , UINT len , UINT flags )
// 
// exception version of send()
// 
// Parameters 
//     s     - socket descriptor
//     buf   - input buffer
//     len   - input data length
//     flags - send flag (MSG_OOB,MSG_DONTROUTE)
// 
// Return 
//     length of bytes sent
// 
// 
//////////////////////////////////////////////////////////////////////
UINT SocketAPI::send_ex ( SOCKET s , const VOID * buf , UINT len , UINT flags )
{
	//__ENTER_FUNCTION_FOXNET

	INT nSent;

	try
	{

#if __LINUX__
	nSent = send(s,buf,len,flags);
#elif _WINDOWS
	nSent = send(s,(const CHAR *)buf,len,flags);
#endif

	if ( nSent == SOCKET_ERROR ) 
	{
#if __LINUX__
		switch ( errno ) 
		{

		case EWOULDBLOCK : 
			return SOCKET_ERROR_WOULDBLOCK;

		case ECONNRESET :
		case EPIPE :

		case EBADF : 
		case ENOTSOCK : 
		case EFAULT : 
		case EMSGSIZE : 
		case ENOBUFS : 

		default : 
			{
				break;
			}
		}//end of switch
#elif _WINDOWS
		INT iErr = WSAGetLastError() ;
		switch ( iErr ) 
		{
		case WSANOTINITIALISED : 
			strncpy_s( Error, "WSANOTINITIALISED", _ESIZE ) ;
			break ;
		case WSAENETDOWN : 
			strncpy_s( Error, "WSAENETDOWN", _ESIZE ) ;
			break ;
		case WSAEACCES : 
			strncpy_s( Error, "WSAEACCES", _ESIZE ) ;
			break ;
		case WSAEINTR : 
			strncpy_s( Error, "WSAEINTR", _ESIZE ) ;
			break ;
		case WSAEINPROGRESS : 
			strncpy_s( Error, "WSAEINPROGRESS", _ESIZE ) ;
			break ;
		case WSAEFAULT : 
			strncpy_s( Error, "WSAEFAULT", _ESIZE ) ;
			break ;
		case WSAENETRESET : 
			strncpy_s( Error, "WSAENETRESET", _ESIZE ) ;
			break ;
		case WSAENOBUFS : 
			strncpy_s( Error, "WSAENOBUFS", _ESIZE ) ;
			break ;
		case WSAENOTCONN : 
			strncpy_s( Error, "WSAENOTCONN", _ESIZE ) ;
			break ;
		case WSAENOTSOCK : 
			strncpy_s( Error, "WSAENOTSOCK", _ESIZE ) ;
			break ;
		case WSAEOPNOTSUPP : 
			strncpy_s( Error, "WSAEOPNOTSUPP", _ESIZE ) ;
			break ;
		case WSAESHUTDOWN : 
			strncpy_s( Error, "WSAESHUTDOWN", _ESIZE ) ;
			break ;
		case WSAEWOULDBLOCK : 
//			strncpy_s( Error, "WSAEWOULDBLOCK", _ESIZE ) ;
			return SOCKET_ERROR_WOULDBLOCK ;
			break ;
		case WSAEMSGSIZE : 
			strncpy_s( Error, "WSAEMSGSIZE", _ESIZE ) ;
			break ;
		case WSAEHOSTUNREACH : 
			strncpy_s( Error, "WSAEHOSTUNREACH", _ESIZE ) ;
			break ;
		case WSAEINVAL : 
			strncpy_s( Error, "WSAEINVAL", _ESIZE ) ;
			break ;
		case WSAECONNABORTED : 
			strncpy_s( Error, "WSAECONNABORTED", _ESIZE ) ;
			break ;
		case WSAECONNRESET : 
			strncpy_s( Error, "WSAECONNRESET", _ESIZE ) ;
			break ;
		case WSAETIMEDOUT : 
			strncpy_s( Error, "WSAETIMEDOUT", _ESIZE ) ;
			break ;
		default :
			{
			strncpy_s( Error, "UNKNOWN", _ESIZE ) ;
			break ;
			};
		};//end of switch
#endif
	} 
	else if ( nSent == 0 )
	{
	}

	}
	catch(...)
	{
	}

	return nSent;
	
	//__LEAVE_FUNCTION_FOXNET

	return 0 ;
}


//////////////////////////////////////////////////////////////////////
// exception version of sendto()
//////////////////////////////////////////////////////////////////////
UINT SocketAPI::sendto_ex ( SOCKET s , const VOID * buf , INT len , UINT flags , const struct sockaddr * to , INT tolen )
{
	//__ENTER_FUNCTION_FOXNET

#if __LINUX__
	INT nSent = sendto(s,buf,len,flags,to,tolen);
#elif _WINDOWS
	INT nSent = sendto(s,(const CHAR *)buf,len,flags,to,tolen);
#endif

	if ( nSent == SOCKET_ERROR ) 
	{
#if __LINUX__
		switch ( errno ) 
		{

		case EWOULDBLOCK : 
			return 0;

		case ECONNRESET :
		case EPIPE :

		case EBADF : 
		case ENOTSOCK : 
		case EFAULT : 
		case EMSGSIZE : 
		case ENOBUFS : 

		default : 
			{
				break;
			}
		}	
#elif _WINDOWS
#endif
	}

	return nSent;

	//__LEAVE_FUNCTION_FOXNET

	return 0 ;
}


//////////////////////////////////////////////////////////////////////
//
// UINT SocketAPI::recv_ex ( SOCKET s , VOID * buf , UINT len , UINT flags )
//
// exception version of recv()
//
// Parameters 
//     s     - socket descriptor
//     buf   - input buffer
//     len   - input data length
//     flags - send flag (MSG_OOB,MSG_DONTROUTE)
// 
// Return 
//     length of bytes received
// 
//
//////////////////////////////////////////////////////////////////////
UINT SocketAPI::recv_ex ( SOCKET s , VOID * buf , UINT len , UINT flags )
{
	//__ENTER_FUNCTION_FOXNET

#if __LINUX__
	INT nrecv = recv(s,buf,len,flags);
#elif _WINDOWS
	INT nrecv = recv(s,(CHAR*)buf,len,flags);
#endif

	if ( nrecv == SOCKET_ERROR ) 
	{
#if __LINUX__
		switch ( errno ) 
		{

		case EWOULDBLOCK : 
			return SOCKET_ERROR_WOULDBLOCK;

		case ECONNRESET :
		case EPIPE :

		case EBADF : 
		case ENOTCONN : 
		case ENOTSOCK : 
		case EINTR : 
		case EFAULT : 

		default : 
			{
				break;
			}
		}//end of switch

#elif _WINDOWS
		INT iErr = WSAGetLastError() ;
		switch ( iErr ) 
		{
		case WSANOTINITIALISED : 
			strncpy_s( Error, "WSANOTINITIALISED", _ESIZE ) ;
			break ;
		case WSAENETDOWN : 
			strncpy_s( Error, "WSAENETDOWN", _ESIZE ) ;
			break ;
		case WSAEFAULT : 
			strncpy_s( Error, "WSAEFAULT", _ESIZE ) ;
			break ;
		case WSAENOTCONN : 
			strncpy_s( Error, "WSAENOTCONN", _ESIZE ) ;
			break ;
		case WSAEINTR : 
			strncpy_s( Error, "WSAEINTR", _ESIZE ) ;
			break ;
		case WSAEINPROGRESS : 
			strncpy_s( Error, "WSAEINPROGRESS", _ESIZE ) ;
			break ;
		case WSAENETRESET : 
			strncpy_s( Error, "WSAENETRESET", _ESIZE ) ;
			break ;
		case WSAENOTSOCK : 
			strncpy_s( Error, "WSAENOTSOCK", _ESIZE ) ;
			break ;
		case WSAEOPNOTSUPP : 
			strncpy_s( Error, "WSAEOPNOTSUPP", _ESIZE ) ;
			break ;
		case WSAESHUTDOWN : 
			strncpy_s( Error, "WSAESHUTDOWN", _ESIZE ) ;
			break ;
		case WSAEWOULDBLOCK : 
//			strncpy_s( Error, "WSAEWOULDBLOCK", _ESIZE ) ;
			return SOCKET_ERROR_WOULDBLOCK ;
			break ;
		case WSAEMSGSIZE : 
			strncpy_s( Error, "WSAEMSGSIZE", _ESIZE ) ;
			break ;
		case WSAEINVAL : 
			strncpy_s( Error, "WSAEINVAL", _ESIZE ) ;
			break ;
		case WSAECONNABORTED : 
			strncpy_s( Error, "WSAECONNABORTED", _ESIZE ) ;
			break ;
		case WSAETIMEDOUT : 
			strncpy_s( Error, "WSAETIMEDOUT", _ESIZE ) ;
			break ;
		case WSAECONNRESET : 
			strncpy_s( Error, "WSAECONNRESET", _ESIZE ) ;
			break ;
		default :
			{
			strncpy_s( Error, "UNKNOWN", _ESIZE ) ;
			break ;
			};
		};//end of switch
#endif
	} 
	else if ( nrecv == 0 )
	{
	}

	return nrecv;
	
	//__LEAVE_FUNCTION_FOXNET

	return 0 ;
}


/////////////////////////////////////////////////////////////////////
// exception version of recvfrom()
/////////////////////////////////////////////////////////////////////
UINT SocketAPI::recvfrom_ex ( SOCKET s , VOID * buf , INT len , UINT flags , struct sockaddr * from , UINT * fromlen )
{
	//__ENTER_FUNCTION_FOXNET

#if __LINUX__
	INT nReceived = recvfrom(s,buf,len,flags,from,fromlen);

#elif _WINDOWS
	INT nReceived = recvfrom(s,(CHAR*)buf,len,flags,from,(int*)fromlen);
#endif

	if ( nReceived == SOCKET_ERROR ) 
	{
#if __LINUX__
		switch ( errno ) 
		{

		case EWOULDBLOCK : 
			return SOCKET_ERROR_WOULDBLOCK;

		case ECONNRESET :
		case EPIPE :

		case EBADF : 
		case ENOTCONN : 
		case ENOTSOCK : 
		case EINTR : 
		case EFAULT : 

		default : 
			{
				break;
			}
		}//end of switch
#elif _WINDOWS
#endif
	}

	return nReceived;

	//__LEAVE_FUNCTION_FOXNET

	return 0 ;
}


/////////////////////////////////////////////////////////////////////
//
// VOID SocketAPI::closesocket_ex ( SOCKET s )
//
// exception version of closesocket()
//
// Parameters
//     s - socket descriptor
//
// Return
//     none
//
//
/////////////////////////////////////////////////////////////////////
BOOL SocketAPI::closesocket_ex ( SOCKET s )
{
	//__ENTER_FUNCTION_FOXNET

#if __LINUX__
	close(s);
#elif _WINDOWS
	if ( closesocket(s) == SOCKET_ERROR ) 
	{
		INT iErr = WSAGetLastError() ;
		switch ( iErr ) 
		{
		case WSANOTINITIALISED : 
			strncpy_s( Error, "WSANOTINITIALISED", _ESIZE ) ;
			break ;
		case WSAENETDOWN : 
			strncpy_s( Error, "WSAENETDOWN", _ESIZE ) ;
			break ;
		case WSAENOTSOCK : 
			strncpy_s( Error, "WSAENOTSOCK", _ESIZE ) ;
			break ;
		case WSAEINPROGRESS : 
			strncpy_s( Error, "WSAEINPROGRESS", _ESIZE ) ;
			break ;
		case WSAEINTR : 
			strncpy_s( Error, "WSAEINTR", _ESIZE ) ;
			break ;
		case WSAEWOULDBLOCK : 
			strncpy_s( Error, "WSAEWOULDBLOCK", _ESIZE ) ;
			break ;
		default : 
			{
			strncpy_s( Error, "UNKNOWN", _ESIZE ) ;
			break ;
			};
		};//end of switch

		return FALSE ;
	}
#endif

	return TRUE ;
	
	//__LEAVE_FUNCTION_FOXNET

	return FALSE ;
}


/////////////////////////////////////////////////////////////////////
//
// VOID SocketAPI::ioctlsocket_ex ( SOCKET s , LONG cmd , ULONG * argp )
//
// exception version of ioctlsocket()
//
/////////////////////////////////////////////////////////////////////
BOOL SocketAPI::ioctlsocket_ex ( SOCKET s , LONG cmd , ULONG * argp )
{
	//__ENTER_FUNCTION_FOXNET

#if __LINUX__
#elif _WINDOWS
	if ( ioctlsocket(s,cmd,argp) == SOCKET_ERROR ) 
	{
		INT iErr = WSAGetLastError() ;
		switch ( iErr ) 
		{
		case WSANOTINITIALISED : 
			strncpy_s( Error, "WSANOTINITIALISED", _ESIZE ) ;
			break ;
		case WSAENETDOWN : 
			strncpy_s( Error, "WSAENETDOWN", _ESIZE ) ;
			break ;
		case WSAEINPROGRESS : 
			strncpy_s( Error, "WSAEINPROGRESS", _ESIZE ) ;
			break ;
		case WSAENOTSOCK : 
			strncpy_s( Error, "WSAENOTSOCK", _ESIZE ) ;
			break ;
		case WSAEFAULT : 
			strncpy_s( Error, "WSAEFAULT", _ESIZE ) ;
			break ;
		default :
			{
			strncpy_s( Error, "UNKNOWN", _ESIZE ) ;
			break ;
			};
		};

		return FALSE ;
	}
#endif

	return TRUE ;
	
	//__LEAVE_FUNCTION_FOXNET

	return FALSE ;
}
 

//////////////////////////////////////////////////////////////////////
//
// BOOL SocketAPI::getsocketnonblocking_ex ( SOCKET s ) 
//
// check if this socket is nonblocking mode
//
// Parameters
//     s - socket descriptor
//
// Return
//     TRUE if nonblocking, FALSE if blocking
//
//
//////////////////////////////////////////////////////////////////////
BOOL SocketAPI::getsocketnonblocking_ex ( SOCKET s )
{
	//__ENTER_FUNCTION_FOXNET

#if __LINUX__
		INT flags = FileAPI::fcntl_ex( s , F_GETFL , 0 );

	return flags | O_NONBLOCK;
#elif _WINDOWS
	return FALSE ;
#endif
	
	//__LEAVE_FUNCTION_FOXNET

	return FALSE ;
}


//////////////////////////////////////////////////////////////////////
//
// VOID SocketAPI::setsocketnonblocking_ex ( SOCKET s , BOOL on ) 
//
// make this socket blocking/nonblocking
//
// Parameters
//     s  - socket descriptor
//     on - TRUE if nonblocking, FALSE if blocking
//
// Return
//     none
//
//////////////////////////////////////////////////////////////////////
BOOL SocketAPI::setsocketnonblocking_ex ( SOCKET s , BOOL on )
{
	//__ENTER_FUNCTION_FOXNET

#if __LINUX__
		INT flags = FileAPI::fcntl_ex( s , F_GETFL , 0 );

	if ( on )
		// make nonblocking fd
		flags |= O_NONBLOCK;
	else
		// make blocking fd
		flags &= ~O_NONBLOCK;

	FileAPI::fcntl_ex( s , F_SETFL , flags );

	return TRUE;
#elif _WINDOWS

	ULONG argp = ( on == TRUE ) ? 1 : 0;
	return ioctlsocket_ex( s,FIONBIO,&argp);

#endif
	
	//__LEAVE_FUNCTION_FOXNET

	return FALSE ;
}

//////////////////////////////////////////////////////////////////////
//
// UINT SocketAPI::availablesocket_ex ( SOCKET s )
//
// get amount of data in socket input buffer
//
// Parameters
//    s - socket descriptor
//
// Return
//    amount of data in socket input buffer
//
//
//////////////////////////////////////////////////////////////////////
UINT SocketAPI::availablesocket_ex ( SOCKET s )
{
	//__ENTER_FUNCTION_FOXNET

#if __LINUX__
		return FileAPI::availablefile_ex(s);
#elif _WINDOWS
	ULONG argp = 0;
	ioctlsocket_ex(s,FIONREAD,&argp);
	return argp;
#endif

	
	//__LEAVE_FUNCTION_FOXNET
	return 0 ;
}


//////////////////////////////////////////////////////////////////////
//
// VOID SocketAPI::shutdown_ex ( SOCKET s , UINT how )
//
// shutdown all or part of connection of socket
//
// Parameters
//     s   - socket descriptor
//     how - how to close ( all , send , receive )
//
// Return
//     none
//
//
//////////////////////////////////////////////////////////////////////
BOOL SocketAPI::shutdown_ex ( SOCKET s , UINT how )
{
	//__ENTER_FUNCTION_FOXNET

	if ( shutdown(s,how) < 0 ) 
	{
#if __LINUX__
		switch ( errno ) {
		case EBADF : 
		case ENOTSOCK : 
		case ENOTCONN : 
		default : 
			{
				break;
			}
		}
#elif _WINDOWS
		INT iErr = WSAGetLastError() ;
		switch ( iErr ) 
		{
		case WSANOTINITIALISED : 
			strncpy_s( Error, "WSANOTINITIALISED", _ESIZE ) ;
			break ;
		case WSAENETDOWN :
			strncpy_s( Error, "WSAENETDOWN", _ESIZE ) ;
			break ;
		case WSAEINVAL : 
			strncpy_s( Error, "WSAEINVAL", _ESIZE ) ;
			break ;
		case WSAEINPROGRESS : 
			strncpy_s( Error, "WSAEINPROGRESS", _ESIZE ) ;
			break ;
		case WSAENOTCONN : 
			strncpy_s( Error, "WSAENOTCONN", _ESIZE ) ;
			break ;
		case WSAENOTSOCK : 
			strncpy_s( Error, "WSAENOTSOCK", _ESIZE ) ;
			break ;
		default :
			{
			strncpy_s( Error, "UNKNOWN", _ESIZE ) ;
			break ;
			};
		};
#endif

		return FALSE ;
	}

	return TRUE ;
	
	//__LEAVE_FUNCTION_FOXNET

	return FALSE ;
}

//////////////////////////////////////////////////////////////////////
//
// INT SocketAPI::select_ex ( INT maxfdp1 , fd_set * readset , fd_set * writeset , fd_set * exceptset , struct timeval * timeout )
//
// system call for I/O multiplexing
//
// Parameters
//     maxfdp1   - 
//     readset   - 
//     writeset  - 
//     exceptset - 
//     timeout   - 
//
// Return
//     positive count of ready descriptors
//
//
//////////////////////////////////////////////////////////////////////
INT SocketAPI::select_ex ( INT maxfdp1 , fd_set * readset , fd_set * writeset , fd_set * exceptset , struct timeval * timeout )
{
	//__ENTER_FUNCTION_FOXNET

	INT result;

	try 
	{
		result = select( maxfdp1 , readset , writeset , exceptset , timeout );
		if( result == SOCKET_ERROR )
		{
#if __LINUX__

#elif _WINDOWS
		INT iErr = WSAGetLastError() ;
		switch ( iErr ) 
		{
		case WSANOTINITIALISED : 
			strncpy_s( Error, "WSANOTINITIALISED", _ESIZE ) ;
			break ;
		case WSAEFAULT:
			strncpy_s( Error, "WSAEFAULT", _ESIZE ) ;
			break ;
		case WSAENETDOWN:
			strncpy_s( Error, "WSAENETDOWN", _ESIZE ) ;
			break ;
		case WSAEINVAL:
			strncpy_s( Error, "WSAEINVAL", _ESIZE ) ;
			break ;
		case WSAEINTR:
			strncpy_s( Error, "WSAEINTR", _ESIZE ) ;
			break ;
		case WSAEINPROGRESS:
			strncpy_s( Error, "WSAEINPROGRESS", _ESIZE ) ;
			break ;
		case WSAENOTSOCK:
			strncpy_s( Error, "WSAENOTSOCK", _ESIZE ) ;
			break ;
		default :
			{
			strncpy_s( Error, "UNKNOWN", _ESIZE ) ;
			break ;
			};
		};
#endif
		}//end if
	} 
	catch(...)
	{
	}

	return result;

	//__LEAVE_FUNCTION_FOXNET

	return 0 ;
}




