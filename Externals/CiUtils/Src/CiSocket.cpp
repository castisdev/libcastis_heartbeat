// CiSocket.cpp: implementation of the CCiSocket class.
//
//////////////////////////////////////////////////////////////////////

#include "internal_CiUtils.h"	/* CiUtils.h includes CiGlobals.h hence windows.h */

#include "CiSocket.h"
#include "NetUtil.h"

#ifndef _WIN32
#include <sys/poll.h>
#endif

#if defined(_WIN32) && defined(_DEBUG)
#define new DEBUG_NEW
#endif

#ifdef _WIN32
#pragma warning(disable : 4244 4389)
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CCiSocket::CCiSocket(CCiSocketType_t socketType, int iSendBufferSize, int iRecvBufferSize)
{
	Initialize(CI_INVALID_SOCKET, socketType);

	m_iSendBufferSize = iSendBufferSize;
	m_iRecvBufferSize = iRecvBufferSize;

	m_pSendBuffer = new char[m_iSendBufferSize];
	m_pRecvBuffer = new char[m_iRecvBufferSize];
}

CCiSocket::CCiSocket(int iAcceptSocket, CCiSocketType_t socketType, int iSendBufferSize, int iRecvBufferSize)
{
	Initialize(iAcceptSocket, socketType);

	m_iSendBufferSize = iSendBufferSize;
	m_iRecvBufferSize = iRecvBufferSize;

	m_pSendBuffer = new char[m_iSendBufferSize];
	m_pRecvBuffer = new char[m_iRecvBufferSize];
}

CCiSocket::~CCiSocket()
{
	Finalize();

	if ( m_pSendBuffer != NULL )
	{
		delete[] m_pSendBuffer;
		m_pSendBuffer = NULL;
	}

	if ( m_pRecvBuffer != NULL )
	{
		delete[] m_pRecvBuffer;
		m_pRecvBuffer = NULL;
	}
}

bool CCiSocket::Initialize(int iAcceptSocket, CCiSocketType_t socketType)
{
	m_iSocket = iAcceptSocket;
	m_socketType = socketType;

	if ( m_iSocket != CI_INVALID_SOCKET )
		SetNonBlockSocket();

	m_iConnectTimeOutMillieSec = CI_SOCKET_DEFAULT_CONNECT_TIME_OUT;
	m_iSendTimeOutMillieSec = CI_SOCKET_DEFAULT_SEND_TIME_OUT;
	m_iRecvTimeOutMillieSec = CI_SOCKET_DEFAULT_RECV_TIME_OUT;

	m_iSendStartPtr = 0;
	m_iSendEndPtr = 0;
	m_iRecvStartPtr = 0;
	m_iRecvEndPtr = 0;

	return true;
}

bool CCiSocket::Disconnect()
{
	if ( m_iSocket == CI_INVALID_SOCKET ) 
	{
		return true;
	}

	bool ret = nu_disconnect(m_iSocket);
	m_iSocket = CI_INVALID_SOCKET;

	return ret;
}

bool CCiSocket::Connect(const char* pszServerIPAddress,
						unsigned short iPortNumber,
						const char *pszSourceIPAddress,
						int iSocketBufferSize)
{
	// create socket
	if ( m_socketType == CI_SOCKET_TCP )
	{
		m_iSocket = socket(AF_INET, SOCK_STREAM, 0);
	}
	else if ( m_socketType == CI_SOCKET_UDP ) 
	{
		m_iSocket = socket(AF_INET, SOCK_DGRAM, 0);
	}

	if ( m_iSocket < 0 )
	{

		return false;
	}

	// nu_set_nonblock
	if ( SetNonBlockSocket() != true )
	{

		return false;
	}

	// sinma - 2006. 04. 05
	// close 시 TIME_WAIT 이 발생하지 않도록 하기 위해
	// linger option 을 사용함. 단, 이 경우 graceful disconnect 가 아닌
	// hard disconnect 가 됨.
	//	다른 최신 소스와의 통일 성을 위해 기존 1,0 => 1,5로 변경 2012.1.6 kty027
	// Non_Blocking Socket 에서 linger 의 Timeout 이 정상 동작 안 하는 이유로 1.0 으로 다시 원복 2012.12.11 dahakan
	linger ling = {1,0};
	setsockopt(m_iSocket, SOL_SOCKET, SO_LINGER, (char*)&ling, sizeof(linger));

	// by gun 2004.5.23
	if ( iSocketBufferSize > 0 )
	{
		// SocketBuffer Size 키우고 Send/Recv 통일...
		int buffersize = iSocketBufferSize;
		socklen_t len = sizeof(buffersize);

		if ( setsockopt( m_iSocket, SOL_SOCKET, SO_RCVBUF, (const char *)&buffersize, len ) < 0 )
		{

			return false;
		}

		if ( setsockopt( m_iSocket, SOL_SOCKET, SO_SNDBUF, (const char *)&buffersize, len ) < 0 )
		{

			return false;
		}
	}

	// SourceIPAddress가 지정되어 있으면 그 IPAddress로 bind
	if ( pszSourceIPAddress && strlen(pszSourceIPAddress) > 0 )
	{
		struct sockaddr_in local_addr;
		memset((void *)&local_addr, 0, sizeof(local_addr));
		local_addr.sin_family = AF_INET;
		local_addr.sin_port = htons(0);
#ifdef _WIN32
		local_addr.sin_addr.S_un.S_addr = inet_addr(pszSourceIPAddress);
#else
		inet_pton(AF_INET, pszSourceIPAddress, &local_addr.sin_addr);
#endif
		bind(m_iSocket, (struct sockaddr *)&local_addr, sizeof(local_addr));
	}

	// nu_connect
	struct sockaddr_in server_address;

	memset((void *)&server_address, 0, sizeof(server_address));
	server_address.sin_family = AF_INET;
	server_address.sin_port = htons(iPortNumber);
#ifdef _WIN32
	server_address.sin_addr.S_un.S_addr = inet_addr(pszServerIPAddress);
#else
	if ( inet_pton(AF_INET, pszServerIPAddress, &server_address.sin_addr) != 1 )
	{

		return false;
	}
#endif

	if ( connect(m_iSocket, (const struct sockaddr *)&server_address, sizeof(server_address)) != 0 )
	{
		int iError = GetLastError();
#ifdef _WIN32
		if ( iError != WSAEWOULDBLOCK )
#else
		if ( iError != EWOULDBLOCK && iError != EINPROGRESS )
#endif
		{
			Disconnect();

			return false;
		}

#ifdef _WIN32
		struct timeval timeout = { m_iConnectTimeOutMillieSec/1000, (m_iConnectTimeOutMillieSec % 1000)*1000 };

		fd_set rset, wset;

		FD_ZERO(&rset);
		FD_SET(m_iSocket, &rset);

		wset = rset;

		int iNEvent = select( m_iSocket+1, &rset, &wset, NULL, &timeout);

		if ( iNEvent < 0 )
		{
			Disconnect();
			return false;
		}
		else if ( iNEvent == 0 )
		{
			Disconnect();
			return false;
		}

		if ( !FD_ISSET( m_iSocket, &rset ) && !FD_ISSET( m_iSocket, &wset ) )
		{
			Disconnect();
			return false;
		}
#else
		pollfd pollfd;
		pollfd.fd = m_iSocket;
		pollfd.events = POLLOUT | POLLWRNORM | POLLWRBAND | POLLERR;
		pollfd.revents = 0;
		int iNEvent = 0;
		do
		{
			iNEvent = poll(&pollfd, 1, m_iConnectTimeOutMillieSec);
		}while(iNEvent < 0 && errno == EINTR);
		
		if ( iNEvent < 0 )
		{
			Disconnect();
			return false;
		}
		else if ( iNEvent == 0 )
		{
			Disconnect();
			return false;
		}
#endif

		int error = 0;
		socklen_t len = sizeof(error);

		if ( getsockopt( m_iSocket, SOL_SOCKET, SO_ERROR, (char *)&error, &len ) < 0 )
		{
			Disconnect();

			return false;
		}

		if ( error )
		{
			Disconnect();

			return false;
		}

		return true;
	}

	return true;
}

bool CCiSocket::SetNonBlockSocket()
{
#ifdef _WIN32
	unsigned long cmd = 1;

	if( ioctlsocket( m_iSocket, FIONBIO, &cmd ) != 0 ) {
		return false;
	}
#else

	int val = fcntl( m_iSocket, F_GETFL, 0 );
	if ( val == -1 ) {
		return false;
	}
	if( fcntl( m_iSocket, F_SETFL, val | O_NONBLOCK ) == -1 ) {
		return false;
	}
#endif

	return true;
}

bool CCiSocket::ReadInt16(short *psInt16)
{
	short int16_temp;
	int nread;

	if ( !ReadN((void *)&int16_temp, 2, &nread) )
	{
		return false;
	}

	if ( nread != 2 )
	{
		return false;
	}

	*psInt16 = ntohs(int16_temp);

	return true;
}

bool CCiSocket::ReadInt32(int* piInt32)
{
	int int32_temp;
	int nread;

	if ( ReadN((void *)&int32_temp, 4, &nread) != true )
	{
		return false;
	}

	if ( nread != 4 )
	{
		return false;
	}

	*piInt32 = ntohl(int32_temp);

	return true;
}

bool CCiSocket::ReadInt64(long long* pllInt64)
{
	long long int64_temp0;
	long long int64_temp1;
	int nread;
	unsigned char *ptr_temp0;
	unsigned char *ptr_temp1;

	if ( ReadN((void *)&int64_temp0, 8, &nread) != true)
	{
		return false;
	}

	if ( nread != 8 )
	{
		return false;
	}

	int64_temp1 = int64_temp0;

	if ( 1 != ntohl(1) )
	{
		int i;

		ptr_temp0 = (unsigned char *)&int64_temp0;
		ptr_temp1 = (unsigned char *)&int64_temp1;

		for ( i = 0; i < (int)sizeof(long long); i++ )
		{
			ptr_temp1[i] = ptr_temp0[sizeof(long long)-1-i];
		}
	}

	*pllInt64 = int64_temp1;

	return true;
}

bool CCiSocket::ReadN(void* pBuf, int ntoread, int* pnread)
{
	char* pBuffer = (char*)pBuf;

	if (( m_iRecvEndPtr > m_iRecvStartPtr )
		&& (( m_iRecvEndPtr - m_iRecvStartPtr ) >= ntoread ))
	{
		char* ptr = m_pRecvBuffer + m_iRecvStartPtr;

		memcpy( pBuffer, ptr, ntoread );

		*pnread = ntoread;

		m_iRecvStartPtr += ntoread;

		return true;
	}
	else if (( m_iRecvEndPtr < m_iRecvStartPtr )
				&& (  m_iRecvBufferSize - ( m_iRecvStartPtr - m_iRecvEndPtr ) >= ntoread ))
	{
		if (( m_iRecvBufferSize - m_iRecvStartPtr ) >= ntoread )
		{
			char* ptr = m_pRecvBuffer + m_iRecvStartPtr;

			memcpy( pBuffer, ptr, ntoread );

			*pnread = ntoread;

			m_iRecvStartPtr += ntoread;

			if ( m_iRecvStartPtr == m_iRecvBufferSize )
				m_iRecvStartPtr = 0;

			return true;
		}
		else
		{
			char* ptr = m_pRecvBuffer + m_iRecvStartPtr;

			memcpy( pBuffer, ptr, ( m_iRecvBufferSize - m_iRecvStartPtr ));

			ptr = m_pRecvBuffer;

			memcpy( pBuffer + ( m_iRecvBufferSize - m_iRecvStartPtr ), ptr, ( ntoread - ( m_iRecvBufferSize - m_iRecvStartPtr )));

			*pnread = ntoread;

			m_iRecvStartPtr = ( ntoread - ( m_iRecvBufferSize - m_iRecvStartPtr ));

			return true;
		}
	}
	else
	{
		return false;
	}
}

bool CCiSocket::LookAheadInt32(int* piInt32, int iOffset)
{
	char* pBuf = (char*)piInt32;

	//	start    -----     end 사이의 데이터를 읽어 가는 상황이고, 읽어야 하는 데이터는 충분하다.
	if (( m_iRecvEndPtr > m_iRecvStartPtr )
		&& (( m_iRecvEndPtr - m_iRecvStartPtr ) >= ( iOffset + 4 )))
	{
		char* ptr = m_pRecvBuffer + m_iRecvStartPtr + iOffset;

		memcpy( pBuf, ptr, 4 );

		*piInt32 = ntohl(*(int*)pBuf);

		return true;
	}
	//	버퍼의 내용은 충분하다.
	else if (( m_iRecvEndPtr < m_iRecvStartPtr )
		&& (  m_iRecvBufferSize - ( m_iRecvStartPtr - m_iRecvEndPtr ) >= ( iOffset + 4 ) ))
	{
		//	읽어야 할 4byte가 오른족에 full로 존재하는 경우
		if (( m_iRecvBufferSize - ( m_iRecvStartPtr + iOffset )) >= 4 )
		{
			char* ptr = m_pRecvBuffer + m_iRecvStartPtr + iOffset;

			memcpy( pBuf, ptr, 4 );

			*piInt32 = ntohl(*(int*)pBuf);

			return true;
		}
		//	읽어야 할 4byte가 쪼개져 있는 경우
		else if( m_iRecvBufferSize - ( m_iRecvStartPtr + iOffset ) > 0)
		{
			char* ptr = m_pRecvBuffer + m_iRecvStartPtr + iOffset;

			memcpy( pBuf, ptr, ( m_iRecvBufferSize - (m_iRecvStartPtr + iOffset)));

			ptr = m_pRecvBuffer;

			memcpy( pBuf + ( m_iRecvBufferSize - (m_iRecvStartPtr + iOffset) ), ptr, ( 4 - ( m_iRecvBufferSize - (m_iRecvStartPtr + iOffset) )));

			*piInt32 = ntohl(*(int*)pBuf);

			return true;
		}
		//	읽어야 할 4byte가 왼쪽에 다 있는 경우, 한 cycle을 넘어선 경우이므로 한 싸이클을 뺀다.
		else
		{
			char* ptr = m_pRecvBuffer + m_iRecvStartPtr + iOffset - m_iRecvBufferSize;

			memcpy( pBuf, ptr, 4);

			*piInt32 = ntohl(*(int*)pBuf);

			return true;
		}
	}
	else
	{
		return false;
	}
}

bool CCiSocket::LookAheadN(void* pBuf, int ntoReadSize, int iOffset)
{
	char* pBuffer = (char*)pBuf;

	//	start    -----     end 사이의 데이터를 읽어 가는 상황이고, 읽어야 하는 데이터는 충분하다.
	if (( m_iRecvEndPtr > m_iRecvStartPtr )
		&& (( m_iRecvEndPtr - m_iRecvStartPtr ) >= ( iOffset + ntoReadSize )))
	{
		char* ptr = m_pRecvBuffer + m_iRecvStartPtr + iOffset;

		memcpy( pBuffer, ptr, ntoReadSize );

		return true;
	}
	//	버퍼의 내용은 충분하다.
	else if (( m_iRecvEndPtr < m_iRecvStartPtr )
		&& (  m_iRecvBufferSize - ( m_iRecvStartPtr - m_iRecvEndPtr ) >= ( iOffset + ntoReadSize ) ))
	{
		//	읽어야 할 4byte가 오른족에 full로 존재하는 경우
		if (( m_iRecvBufferSize - ( m_iRecvStartPtr + iOffset )) >= ntoReadSize )
		{
			char* ptr = m_pRecvBuffer + m_iRecvStartPtr + iOffset;

			memcpy( pBuffer, ptr, ntoReadSize );

			return true;
		}
		//	읽어야 할 4byte가 쪼개져 있는 경우
		else if( m_iRecvBufferSize - ( m_iRecvStartPtr + iOffset ) > 0)
		{
			char* ptr = m_pRecvBuffer + m_iRecvStartPtr + iOffset;

			memcpy( pBuffer, ptr, ( m_iRecvBufferSize - (m_iRecvStartPtr + iOffset)));

			ptr = m_pRecvBuffer;

			memcpy( pBuffer + ( m_iRecvBufferSize - (m_iRecvStartPtr + iOffset) ), ptr, ( ntoReadSize - ( m_iRecvBufferSize - (m_iRecvStartPtr + iOffset) )));

			return true;
		}
		//	읽어야 할 4byte가 왼쪽에 다 있는 경우, 한 cycle을 넘어선 경우이므로 한 싸이클을 뺀다.
		else
		{
			char* ptr = m_pRecvBuffer + m_iRecvStartPtr + iOffset - m_iRecvBufferSize;

			memcpy( pBuffer, ptr, ntoReadSize);

			return true;
		}
	}
	else
	{
		return false;
	}

}

// by gun 2005. 1.26
// parameter로 네트웍에서 읽을 MAX값을 넘겨 받는 함수 추가
bool CCiSocket::ReadFromNetworkToBuffer(int iReadSize)
{
	if ( iReadSize >= ( m_iRecvBufferSize - GetCurrentRecvDataSize() )) 
	{
		return false;
	}

	int nleft = 0;
	int nleft2 = 0;
	int nread_temp;
	char *ptr2 = NULL;

	if ( m_iRecvEndPtr == m_iRecvBufferSize )
		m_iRecvEndPtr = 0;

	char* ptr = m_pRecvBuffer + m_iRecvEndPtr ;

	if ( m_iRecvStartPtr < m_iRecvEndPtr )
	{
		nleft = m_iRecvBufferSize - m_iRecvEndPtr;

		if ( m_iRecvStartPtr == 0 )
			nleft--;

		if ( nleft > iReadSize )
			nleft = iReadSize;
		else
		{
			nleft2 = iReadSize - nleft;
			ptr2 = m_pRecvBuffer;
		}

	}
	else if ( m_iRecvStartPtr > m_iRecvEndPtr )
	{
		nleft = iReadSize;
	}
	else	// ==
	{
		m_iRecvStartPtr = 0;
		m_iRecvEndPtr = 0;
		ptr = m_pRecvBuffer;
		nleft = iReadSize;
	}

	while ( nleft > 0 )
	{
#ifdef _WIN32
		if ( (nread_temp = recv(m_iSocket, ptr, nleft, 0)) < 0 )
#else
		if ( (nread_temp = recv(m_iSocket, ptr, nleft, MSG_NOSIGNAL)) < 0 )
#endif
		{
			int iError = GetLastError();
			if ( iError == CI_EWOULDBLOCK )
			{
				return true;
			}
			else
			{
				return false;
			}
		}
		else if (nread_temp == 0)
		{
			break;	/* EOF */
		}

		nleft -= nread_temp;
		ptr += nread_temp;

		m_iRecvEndPtr += nread_temp;
		if ( m_iRecvEndPtr == m_iRecvBufferSize )
			m_iRecvEndPtr = 0;
	}

	while ( nleft2 > 0 )
	{
#ifdef _WIN32
		if ( (nread_temp = recv(m_iSocket, ptr2, nleft2, 0)) < 0 )
#else
		if ( (nread_temp = recv(m_iSocket, ptr2, nleft2, MSG_NOSIGNAL)) < 0 )
#endif
		{
			int iError = GetLastError();
			if ( iError == CI_EWOULDBLOCK )
			{
				return true;
			}
			else
			{
				return false;
			}
		}
		else if (nread_temp == 0)
		{
			break;	/* EOF */
		}

		nleft2 -= nread_temp;
		ptr2 += nread_temp;

		m_iRecvEndPtr += nread_temp;
	}

	return true;
}

bool CCiSocket::ReadFromNetworkToBuffer()
{
	int nleft;
	int nread_temp;
	char *ptr;

	if ( m_iRecvEndPtr == m_iRecvBufferSize )
		m_iRecvEndPtr = 0;

	ptr = m_pRecvBuffer + m_iRecvEndPtr ;

	if ( m_iRecvStartPtr < m_iRecvEndPtr )
	{
		nleft = m_iRecvBufferSize - m_iRecvEndPtr;
		if ( m_iRecvStartPtr == 0 )
			nleft--;
	}
	else if ( m_iRecvStartPtr > m_iRecvEndPtr )
	{
		nleft = m_iRecvStartPtr - m_iRecvEndPtr - 1;
	}
	else	// ==
	{
		m_iRecvStartPtr = 0;
		m_iRecvEndPtr = 0;
		ptr = m_pRecvBuffer;
		nleft = m_iRecvBufferSize - 1;
	}

	while ( nleft > 0 )
	{
#ifdef _WIN32
		if ( (nread_temp = recv(m_iSocket, ptr, nleft, 0)) < 0 )
#else
		if ( (nread_temp = recv(m_iSocket, ptr, nleft, MSG_NOSIGNAL)) < 0 )
#endif
		{
			int iError = GetLastError();
			if ( iError == CI_EWOULDBLOCK )
			{
				return true;
			}
			else
			{
				return false;
			}
		}
		else if (nread_temp == 0)
		{
			break;	/* EOF */
		}

		nleft -= nread_temp;
		ptr += nread_temp;

		m_iRecvEndPtr += nread_temp;
		if ( m_iRecvEndPtr == m_iRecvBufferSize )
			m_iRecvEndPtr = 0;
	}

	return true;
}

int	CCiSocket::GetCurrentRecvDataSize()
{
	if ( m_iRecvEndPtr > m_iRecvStartPtr )
	{
		return ( m_iRecvEndPtr - m_iRecvStartPtr );
	}
	else if ( m_iRecvEndPtr < m_iRecvStartPtr )
	{
		return ( m_iRecvBufferSize - ( m_iRecvStartPtr - m_iRecvEndPtr ) );
	}

	return 0;
}

int	CCiSocket::GetRemainSendDataSize()
{
	if ( m_iSendEndPtr > m_iSendStartPtr )
	{
		return ( m_iSendEndPtr - m_iSendStartPtr );
	}
	else if ( m_iSendEndPtr < m_iSendStartPtr )
	{
		return ( m_iSendBufferSize - ( m_iSendStartPtr - m_iSendEndPtr ) );
	}

	return 0;
}

bool CCiSocket::WriteInt16(short sInt16)
{
	short int16_temp;
	int nwritten;

	int16_temp = htons(sInt16);
	if ( !WriteN((const void *)&int16_temp, 2, &nwritten) )
	{
		return false;
	}

	if ( nwritten != 2 )
	{
		return false;
	}

	return true;
}

bool CCiSocket::WriteInt32(int iInt32)
{
	int int32_temp;
	int nwritten;

	int32_temp = htonl(iInt32);
	if ( WriteN((const void *)&int32_temp, 4, &nwritten) != true )
	{
		return false;
	}

	if ( nwritten != 4 )
	{
		return false;
	}

	return true;
}

bool CCiSocket::WriteInt64(long long llInt64)
{
	long long int64_temp;
	int nwritten;
	unsigned char *ptr;
	unsigned char *ptr_temp;

	int64_temp = llInt64;
	if ( 1 != htonl(1) )
	{
		int i;

		ptr = (unsigned char *)&llInt64;
		ptr_temp = (unsigned char *)&int64_temp;

		for ( i = 0; i < (int)sizeof(long long); i++ )
		{
			ptr_temp[i] = ptr[sizeof(long long)-1-i];
		}
	}

	if ( WriteN((const void *)&int64_temp, 8, &nwritten) != true )
	{
		return false;
	}

	if ( nwritten != 8 )
	{
		return false;
	}

	return true;
}

bool CCiSocket::WriteN(const void* pBuf, int ntowrite, int* pnwritten)
{
	if ( GetRemainSendDataSize() > 0 )
	{
		if ( WriteFromBufferToNetwork() != true )
		{
			return false;
		}
	}

	if ( GetRemainSendDataSize() > 0 )
	{
		if ( WriteNToBuffer( pBuf, ntowrite, pnwritten ) != true )
		{
			return false;
		}
	}
	else
	{
		int nleft;
		int nwritten_temp;
		const char *ptr;

		ptr = (const char *)pBuf;
		nleft = ntowrite;
		while (nleft > 0)
		{
#ifdef _WIN32
			if ( ( nwritten_temp = send(m_iSocket, ptr, nleft, 0) ) < 0 )
#else
			if ( ( nwritten_temp = send(m_iSocket, ptr, nleft, MSG_NOSIGNAL) ) < 0 )
#endif
			{
				int iError = GetLastError();
				if ( iError == CI_EWOULDBLOCK )
				{
					if ( WriteNToBuffer( ptr, nleft, pnwritten ) != true ) 
					{
						return false;
					}

					*pnwritten = ntowrite;

					return true;
				}
				else
				{
					return false;
				}
			}
			else if ( nwritten_temp == 0 )
			{
				break;
			}
			nleft -= nwritten_temp;
			ptr += nwritten_temp;
		}

		*pnwritten = ntowrite - nleft;

		if( nleft != 0 )
		{
			return false;
		}
	}

	return true;
}

bool CCiSocket::WriteNToBuffer(const void* pBuf, int ntowrite, int* pnwritten)
{
	if ( m_iSendStartPtr == m_iSendEndPtr )
	{
		m_iSendStartPtr = 0;
		m_iSendEndPtr = 0;
	}

	char* pBuffer = (char*)pBuf;

	if (( m_iSendStartPtr > m_iSendEndPtr )
		&& (( m_iSendStartPtr - m_iSendEndPtr ) > ntowrite ))
	{
		char* ptr = m_pSendBuffer + m_iSendEndPtr;

		memcpy( ptr, pBuffer, ntowrite );

		*pnwritten = ntowrite;

		m_iSendEndPtr += ntowrite;

		return true;
	}
	else if (( m_iSendStartPtr <= m_iSendEndPtr )
				&& (  m_iSendBufferSize - ( m_iSendEndPtr - m_iSendStartPtr ) > ntowrite ))
	{
		if (( m_iSendBufferSize - m_iSendEndPtr ) >= ntowrite )
		{
			char* ptr = m_pSendBuffer + m_iSendEndPtr;

			memcpy( ptr, pBuffer, ntowrite );

			*pnwritten = ntowrite;

			m_iSendEndPtr += ntowrite;

			if ( m_iSendEndPtr == m_iSendBufferSize )
				m_iSendEndPtr = 0;

			return true;
		}
		else
		{
			char* ptr = m_pSendBuffer + m_iSendEndPtr;

			memcpy( ptr, pBuffer, ( m_iSendBufferSize - m_iSendEndPtr ));

			ptr = m_pSendBuffer;

			memcpy( ptr, pBuffer + ( m_iSendBufferSize - m_iSendEndPtr ), ( ntowrite - ( m_iSendBufferSize - m_iSendEndPtr )));

			*pnwritten = ntowrite;

			m_iSendEndPtr = ( ntowrite - ( m_iSendBufferSize - m_iSendEndPtr ));

			return true;
		}
	}
	else
	{
		return false;
	}
}

bool CCiSocket::WriteFromBufferToNetwork()
{
	if ( GetRemainSendDataSize() == 0 )
		return true;

	int nleft;
	int nwritten_temp;
	const char *ptr;

	ptr = (const char *)m_pSendBuffer + m_iSendStartPtr;
	if ( m_iSendEndPtr > m_iSendStartPtr )
		nleft = m_iSendEndPtr - m_iSendStartPtr;
	else if ( m_iSendEndPtr < m_iSendStartPtr )
		nleft = m_iSendBufferSize - m_iSendStartPtr;
	else
		nleft = 0;

	while (nleft > 0)
	{
#ifdef _WIN32
		if ( ( nwritten_temp = send(m_iSocket, ptr, nleft, 0) ) < 0 )
#else
		if ( ( nwritten_temp = send(m_iSocket, ptr, nleft, MSG_NOSIGNAL) ) < 0 )
#endif
		{
			int iError = GetLastError();
			if ( iError == CI_EWOULDBLOCK )
			{
				return true;
			}
			else
			{
				return false;
			}
		}

		nleft -= nwritten_temp;
		ptr += nwritten_temp;

		m_iSendStartPtr += nwritten_temp;
		if ( m_iSendStartPtr == m_iSendBufferSize )
			m_iSendStartPtr = 0;
	}
	return true;
}

std::string CCiSocket::GetPeerIP() const
{
	struct sockaddr_in peer_addr;
	socklen_t addrlen = sizeof(struct sockaddr_in);

	if ( getpeername(m_iSocket, (struct sockaddr *)&peer_addr, &addrlen) < 0 )
		return "";
	else
		return inet_ntoa(peer_addr.sin_addr);
}


