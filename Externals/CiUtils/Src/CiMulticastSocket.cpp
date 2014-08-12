// MulticastSocket.cpp: implementation of the CCiMulticastSocket class.
//
//////////////////////////////////////////////////////////////////////
#include "internal_CiUtils.h"	/* CiUtils.h includes CiGlobals.h hence windows.h */

#include "NetUtil.h"
#include "CiMulticastSocket.h"

#include "CiLogger.h"

#if defined(_WIN32) && defined(_DEBUG)
#define new DEBUG_NEW
#endif

#ifdef _WIN32
#pragma warning(disable : 4244 4389)
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CCiMulticastSocket::CCiMulticastSocket()
{
	m_bForceNoLoopback = false;
	m_iSendSocket	 = NU_INVALID_SOCKET;
	m_iReceiveSocket = NU_INVALID_SOCKET;

	m_iSendBufferPosition = 0;
	m_iRecvBufferPosition = 0;
	m_iNBytesReceived = 0;
}

CCiMulticastSocket::~CCiMulticastSocket()
{
	if( m_iSendSocket != NU_INVALID_SOCKET )
	{
		StopSending();
		m_iSendSocket = NU_INVALID_SOCKET;
	}
	if( m_iReceiveSocket != NU_INVALID_SOCKET )
	{
		StopReceiving();
		m_iReceiveSocket = NU_INVALID_SOCKET;
	}

}

/////////////////////////////////////////////////////////////////////////////
// CCiMulticastSocket member functions


//  create multicast socket To receive packets
bool CCiMulticastSocket::CreateBlockingReceiveSocket( const char *groupIP, unsigned short groupPort )
{
	if( ( m_iReceiveSocket = socket( AF_INET, SOCK_DGRAM, 0))  == -1 ) {
		return false;
    }

	// Make receive socket reuseable immediately when it is closed.
	const int on = 1;
	if( setsockopt( m_iReceiveSocket, SOL_SOCKET, SO_REUSEADDR, (char *)&on, sizeof(on)) == -1 ) {
        return false;
    }

	// Fill m_saHostGroup_in for receiving socket */
	memset(&m_saSourceAddress, 0, sizeof(m_saSourceAddress));

	m_saSourceAddress.sin_family = AF_INET;
	m_saSourceAddress.sin_addr.s_addr = htonl(INADDR_ANY);
	m_saSourceAddress.sin_port = htons(groupPort);

	if ( bind( m_iReceiveSocket, (struct sockaddr *)&m_saSourceAddress, sizeof(m_saSourceAddress)) != 0) {
        return false;
    }

	// Join the multicast group
	m_mrMReq.imr_multiaddr.s_addr = inet_addr(groupIP);	/* group addr */
	m_mrMReq.imr_interface.s_addr = INADDR_ANY;		/* use default */

	if( setsockopt(m_iReceiveSocket, IPPROTO_IP, IP_ADD_MEMBERSHIP, (char *)&m_mrMReq, sizeof(m_mrMReq)) < 0 ) {
		return false;
	}

	return true;
}

//   Create multicast socket for Sending Packets    //
bool CCiMulticastSocket::CreateBlockingSendSocket( const char *groupIP, unsigned short groupPort, const char *SourceInterfaceIP, unsigned int nTTL, bool bLoopBack )
{
	// Create socket for Sending packets from multicast group
	if( ( m_iSendSocket = socket(AF_INET, SOCK_DGRAM, 0))  == -1 ) {
		return false;
    }

	// Fill m_saHostGroup for sending datagrams When you use SendN()
	memset(&m_saHostGroup, 0, sizeof(m_saHostGroup));

	m_saHostGroup.sin_family = AF_INET;
	m_saHostGroup.sin_addr.s_addr = inet_addr(groupIP);
	m_saHostGroup.sin_port = htons(groupPort);

	// Fill m_saSourceAddress To bind sending socket( Requirement )
	memset(&m_saSourceAddress, 0, sizeof(m_saSourceAddress));

	m_saSourceAddress.sin_family = AF_INET;
	m_saSourceAddress.sin_addr.s_addr = htonl(INADDR_ANY);
	m_saSourceAddress.sin_port = htons(0);

	if ( bind( m_iSendSocket, (struct sockaddr *)&m_saSourceAddress, sizeof(m_saSourceAddress)) != 0) {
        return false;
    }

	// Make send socket reuseable immediately when it is closed.
	const int on = 1;
	if( setsockopt( m_iSendSocket, SOL_SOCKET, SO_REUSEADDR, (char *)&on, sizeof(on)) == -1 ) {
        return false;
    }

	// Join the multicast group
	m_mrMReq.imr_multiaddr.s_addr = inet_addr(groupIP);	/* group addr */
	if ( strcmp("0.0.0.0", SourceInterfaceIP) == 0 ) {
		m_mrMReq.imr_interface.s_addr = htons(INADDR_ANY);		// use default
	} else {
		m_mrMReq.imr_interface.s_addr = inet_addr(SourceInterfaceIP);
	}// m_mrMReq.imr_interface.S_un.S_addr를 안 주게 되면 IP_ADD_MEMBERSHIP에서 ERROR가 난다.

	int resultOfsetsockopt;
	if( ( resultOfsetsockopt = setsockopt( m_iSendSocket, IPPROTO_IP, IP_ADD_MEMBERSHIP, (char *)&m_mrMReq, sizeof(m_mrMReq) ) ) < 0 ) {
		return false;
	}

	// Local Interface Setting
	if ( strcmp("0.0.0.0", SourceInterfaceIP) == 0 ) {
		m_MyInterfaceAddress.s_addr = htons(INADDR_ANY);  // Use default interface
	}
	else {
		m_MyInterfaceAddress.s_addr = inet_addr(SourceInterfaceIP);
	}

	if( setsockopt( m_iSendSocket, IPPROTO_IP, IP_MULTICAST_IF, (char *)&m_MyInterfaceAddress, sizeof(m_MyInterfaceAddress)) < 0 ) {
		return false;
	}

	if ( !SetTTL(nTTL) ) {// Set Time to Live as specified by user
		return false;
	}

	SetLoopBack(bLoopBack);							// Enable/Disable Loopback

	return true;
}

bool CCiMulticastSocket::CreateNonBlockingReceiveSocket(const char *groupIP, unsigned short groupPort)
{
	if( ( m_iReceiveSocket = socket( AF_INET, SOCK_DGRAM, 0))  == -1 ) {
		return false;
    }

	memset(&m_saSourceAddress, 0, sizeof(m_saSourceAddress));

	m_saSourceAddress.sin_family = AF_INET;
	m_saSourceAddress.sin_addr.s_addr = htonl(INADDR_ANY);
	m_saSourceAddress.sin_port = htons(groupPort);

	if ( bind( m_iReceiveSocket, (struct sockaddr *)&m_saSourceAddress, sizeof(m_saSourceAddress)) != 0 ) {
		return false;
    }

	// Make send socket reuseable immediately when it is closed.
	const int on = 1;
	if ( setsockopt( m_iReceiveSocket, SOL_SOCKET, SO_REUSEADDR, (char *)&on, sizeof(on)) == -1 ) {
        return false;
    }

	// Join the multicast group
	m_mrMReq.imr_multiaddr.s_addr = inet_addr(groupIP);	/* group addr */
	m_mrMReq.imr_interface.s_addr = htons(INADDR_ANY);		/* use default */

	if ( setsockopt(m_iReceiveSocket, IPPROTO_IP, IP_ADD_MEMBERSHIP, (char *)&m_mrMReq, sizeof(m_mrMReq)) < 0 ) {
		return false;
	}

	SetReceiveSocketNonBlocking();

	return true;
}

bool CCiMulticastSocket::CreateNonBlockingSendSocket(const char *groupIP, unsigned short groupPort, const char *SourceInterfaceIP, unsigned int nTTL, bool bLoopBack)
{
	// Create socket for Sending packets from multicast group
	if( ( m_iSendSocket = socket(AF_INET, SOCK_DGRAM, 0))  == -1 ) {
		return false;
    }

	// Fill m_saHostGroup for sending datagrams When you use SendN()
	memset(&m_saHostGroup, 0, sizeof(m_saHostGroup));

	m_saHostGroup.sin_family = AF_INET;
	m_saHostGroup.sin_addr.s_addr = inet_addr(groupIP);
	m_saHostGroup.sin_port = htons(groupPort);

	// Fill m_saSourceAddress To bind sending socket( Requirement )
	memset(&m_saSourceAddress, 0, sizeof(m_saSourceAddress));

	m_saSourceAddress.sin_family = AF_INET;
	m_saSourceAddress.sin_addr.s_addr = htonl(INADDR_ANY);
	m_saSourceAddress.sin_port = htons(0);

	if ( bind( m_iSendSocket, (struct sockaddr *)&m_saSourceAddress, sizeof(m_saSourceAddress)) != 0 ) {
        return false;
    }

	// Make send socket reuseable immediately when it is closed.
	const int on = 1;
	if ( setsockopt( m_iSendSocket, SOL_SOCKET, SO_REUSEADDR, (char *)&on, sizeof(on)) == -1 ) {
        return false;
    }

	// Join the multicast group
	m_mrMReq.imr_multiaddr.s_addr = inet_addr(groupIP);	/* group addr */

	if ( strcmp("0.0.0.0", SourceInterfaceIP) == 0 ) {
		m_mrMReq.imr_interface.s_addr = htons(INADDR_ANY);		// use default
	} else {
		m_mrMReq.imr_interface.s_addr = inet_addr(SourceInterfaceIP);
	}// m_mrMReq.imr_interface.S_un.S_addr를 안 주게 되면 IP_ADD_MEMBERSHIP에서 ERROR가 난다.

	if ( setsockopt( m_iSendSocket, IPPROTO_IP, IP_ADD_MEMBERSHIP, (char *)&m_mrMReq, sizeof(m_mrMReq)) < 0 ) {
		return false;
	}

	// Local Interface Setting
	if ( strcmp("0.0.0.0", SourceInterfaceIP) == 0 ) {
		m_MyInterfaceAddress.s_addr = htons(INADDR_ANY);  // Use default interface
	}
	else {
		m_MyInterfaceAddress.s_addr = inet_addr(SourceInterfaceIP);
	}

	if ( setsockopt( m_iSendSocket, IPPROTO_IP, IP_MULTICAST_IF, (char *)&m_MyInterfaceAddress, sizeof(m_MyInterfaceAddress)) < 0 ) {
		return false;
	}

	if ( !SetTTL(nTTL) ) {// Set Time to Live as specified by user
		return false;
	}

	SetLoopBack(bLoopBack);							// Enable/Disable Loopback
	SetSendSocketNonBlocking();
	return true;
}

bool CCiMulticastSocket::StopReceiving()
{
	if ( setsockopt ( m_iReceiveSocket, IPPROTO_IP, IP_DROP_MEMBERSHIP, (char *)&m_mrMReq, sizeof(m_mrMReq)) < 0 ) {
#ifdef INC_OLE2
		closesocket( m_iReceiveSocket );
#else
		nu_disconnect(m_iReceiveSocket);
#endif
		m_iReceiveSocket = NU_INVALID_SOCKET;
		return false;
	}
	else {
#ifdef INC_OLE2
		closesocket( m_iReceiveSocket );
#else
		nu_disconnect(m_iReceiveSocket);
#endif
		m_iReceiveSocket = NU_INVALID_SOCKET;
		return true;
	}
}

bool CCiMulticastSocket::StopSending()
{
	if ( setsockopt ( m_iSendSocket, IPPROTO_IP, IP_DROP_MEMBERSHIP, (char *)&m_mrMReq, sizeof(m_mrMReq)) < 0 ) {
#ifdef INC_OLE2
		closesocket( m_iSendSocket );
#else
		nu_disconnect(m_iSendSocket);
#endif
		m_iSendSocket = NU_INVALID_SOCKET;
		return false;
	}
	else {
#ifdef INC_OLE2
		closesocket( m_iSendSocket );
#else
		nu_disconnect(m_iSendSocket);
#endif
		m_iSendSocket = NU_INVALID_SOCKET;
		return true;
	}
}

bool CCiMulticastSocket::SetTTL( unsigned int nTTL)
{
	// Set Time to Live to parameter TTL
	if ( setsockopt( m_iSendSocket, IPPROTO_IP, IP_MULTICAST_TTL, (const char *)&nTTL, sizeof(nTTL)) != 0)
		return false;		// Error Setting TTL
	else
		return true;		// else TTL set successfully
}

void CCiMulticastSocket::SetLoopBack( bool bLoop )
{
	// Set LOOPBACK option to true OR FALSE according to IsLoop parameter
	if ( setsockopt( m_iSendSocket, IPPROTO_IP, IP_MULTICAST_LOOP, (const char *)&bLoop, sizeof(bLoop)) != 0 ) {
		// if required to stop loopback
		if ( !bLoop ) {
			m_bForceNoLoopback = true;		// Internally making a note that loopback has to be disabled forcefilly

			// Get IP/Port for send socket in order to disable loopback forcefully
			char localHost[255];
			gethostname(localHost, 255);
			struct hostent *host = gethostbyname(localHost);	// Get local host IP
			strcpy( m_strLocalIP,  inet_ntoa (*(struct in_addr*)*host->h_addr_list));
		}
	}
}

bool CCiMulticastSocket::ReceiveN( char *pReceiveBuffer, int iNBytesToReceive, int *piNReceivedDateBytes )
{
	socklen_t iLengthOfAddress = sizeof( m_saHostGroup );
	int iNReceivedDataBytes;

	while( true )
	{// while로 도는 이유는 WSAEINTR/WSAEWOULDBLOCK의 경우를 위한것임.
		if ( ( iNReceivedDataBytes = recvfrom(  m_iReceiveSocket, pReceiveBuffer, iNBytesToReceive, 0, (struct sockaddr *)&m_saHostGroup, &iLengthOfAddress ) ) == -1 )
		{
			if( nu_get_last_error() == CI_EINTR )
			{
				iNReceivedDataBytes = 0;
				continue;
			}
			else if ( nu_get_last_error() == CI_EWOULDBLOCK )
			{
				iNReceivedDataBytes = 0;
				continue;
			}
			else
			{
				return false;
			}
		}
		else if( iNReceivedDataBytes == 0 ) {
			/* EOF */
			break;
		}
		else {
			break;
		}
	}

	*piNReceivedDateBytes = iNReceivedDataBytes;
	return true;
}

bool CCiMulticastSocket::ReceiveInt32(int &iReceivedInt32Message)
{
	int iTempInt32Message;
	int iReceivedBytes;

	if ( !ReceiveN( (char *)&iTempInt32Message, sizeof(int), &iReceivedBytes ) ) {
		return false;
	}

	iReceivedInt32Message = ntohl( iTempInt32Message );

	return true;
}

bool CCiMulticastSocket::SendN(const void* pBuffer, int iNSizeOfBuffer )
{
	int nWritten;
	int iNLeftBytes;
	const unsigned char *movedOffsetOfSendBuffer;

	iNLeftBytes = iNSizeOfBuffer;
	movedOffsetOfSendBuffer = (const unsigned char *)pBuffer;

	while ( iNLeftBytes > 0 ) {
		if( ( nWritten =
				sendto( m_iSendSocket, (const char *)movedOffsetOfSendBuffer, iNSizeOfBuffer, 0, (struct sockaddr*)&m_saHostGroup, sizeof(m_saHostGroup)))
								== -1) {
			int iError = nu_get_last_error();
			if ( iError == CI_EINTR ) {
				nWritten = 0;
				continue;
			}
			else {
                return false;
			}
		}
		else {
			iNLeftBytes -= nWritten;
			movedOffsetOfSendBuffer +=nWritten;
		}
	}
	return true;
}

bool CCiMulticastSocket::SendInt32(unsigned int uiValue)
{
	unsigned int uiValueToSend = htonl(uiValue);

	if ( !SendN( &uiValueToSend, sizeof( int ) ) ) {
		return false;
	}

	return true;
}

bool CCiMulticastSocket::SetPriority( int priority )
{
	//priority = 20 ;
	if ( setsockopt( m_iSendSocket, IPPROTO_IP , IP_TOS , (char*)&priority , sizeof(priority) ) !=0 )
		return false;
	else return true;
}

bool CCiMulticastSocket::SetSendSocketNonBlocking()
{
	return true;
}

bool CCiMulticastSocket::SetReceiveSocketNonBlocking()
{
#ifdef _WIN32
	unsigned long cmd = 1;
	if( ioctlsocket( m_iReceiveSocket, FIONBIO, &cmd ) != 0 )
		return false;
#else
	int val = fcntl( m_iReceiveSocket, F_GETFL, 0 );
	if ( val == -1 )
		return false;
	if( fcntl( m_iReceiveSocket, F_SETFL, val | O_NONBLOCK ) == -1 )
		return false;
#endif
	return true;
}

bool CCiMulticastSocket::SendNWithTimeOut( unsigned int /*uiSec*/ )
{
	return true;
}

bool CCiMulticastSocket::ReceiveNWithTimeOut( char *pReceiveBuffer, int iNBytesToReceive, int *piNReceivedDateBytes, unsigned int uiSec )
{
	socklen_t iLengthOfAddress = sizeof( m_saHostGroup );
	int iNReceivedDataBytes;
	unsigned int nMilliSecSleepTime = uiSec*1000/10; //10회를 나누어 sleep한다.
	unsigned int uiSleepCount = 0 ;

	while( true ) {
		if ( ( iNReceivedDataBytes = recvfrom(  m_iReceiveSocket, pReceiveBuffer, iNBytesToReceive, 0, (struct sockaddr *)&m_saHostGroup, &iLengthOfAddress ) ) == -1 ) {
			if ( nu_get_last_error() == CI_EINTR ) {
				iNReceivedDataBytes = 0;
				continue;
			}
			else if ( nu_get_last_error() == CI_EWOULDBLOCK ) {
				if( uiSleepCount >= 10 )
				{
					return false;
				}
				castis::msleep( nMilliSecSleepTime );
				uiSleepCount++;
				iNReceivedDataBytes = 0;
				continue;
			} else {
				return false;
			}
		}
		else if( iNReceivedDataBytes == 0 ) {
			/* EOF */
			break;
		}
		else {
			break;
		}
	}

	*piNReceivedDateBytes = iNReceivedDataBytes;
	return true;
}

bool CCiMulticastSocket::SetTimeOutForReceiveSocket( unsigned int uiSec, unsigned int uiUsec )
{
	struct timeval tv;
	int len;

	tv.tv_sec	= uiSec; //2초
	tv.tv_usec	= uiUsec;

	len = sizeof(tv);

	if ( setsockopt( m_iReceiveSocket, SOL_SOCKET, SO_RCVTIMEO, (char *)&tv, len) == -1 ) {
		return false;
	}

	return true;
}

bool CCiMulticastSocket::SetReceiveSocketBufferSize( int nBufferSize )
{
	socklen_t iSize;
	socklen_t len = sizeof(iSize);

	if ( getsockopt(	m_iReceiveSocket,
						SOL_SOCKET,
						SO_RCVBUF,
						(char *)&iSize,
						&len  ) == -1 ) {
		return false;
	}

	socklen_t nReceiveBufferSize = (socklen_t)nBufferSize;

	if ( setsockopt(	m_iReceiveSocket,
						SOL_SOCKET,
						SO_RCVBUF,
						( char * )&nReceiveBufferSize,
						sizeof(nReceiveBufferSize)  ) == -1 ) {
		return false;
	}

	iSize = 0;
	len = sizeof(iSize);

	if ( getsockopt(	m_iReceiveSocket,
						SOL_SOCKET,
						SO_RCVBUF,
						(char *)&iSize,
						&len  ) == -1 ) {
		return false;
	}

	return true;
}

bool CCiMulticastSocket::SetSendSocketBufferSize(int nBufferSize)
{
	socklen_t iSize;
	socklen_t len = sizeof(iSize);

	if( getsockopt(		m_iSendSocket,
						SOL_SOCKET,
						SO_SNDBUF,
						(char *)&iSize,
						&len  ) == -1 ) {
		return false;
	}

	socklen_t nSendBufferSize = (socklen_t)nBufferSize;

	if ( setsockopt(		m_iSendSocket,
						SOL_SOCKET,
						SO_SNDBUF,
						( char * )&nSendBufferSize,
						sizeof(nSendBufferSize)  ) == -1 ) {
		return false;
	}

	iSize = 0;
	len = sizeof(iSize);

	if( getsockopt(		m_iSendSocket,
						SOL_SOCKET,
						SO_SNDBUF,
						(char *)&iSize,
						&len  ) == -1 ) {
		return false;
	}

	return true;
}

// buffering
bool CCiMulticastSocket::FillRecvBufferFromNetwork()
{
	if ( !ReceiveN( (char *)m_recvBuffer, sizeof(m_recvBuffer), (int *)(&m_iNBytesReceived) ) ) {
		return false;
	}

	m_iRecvBufferPosition = 0;

	return true;
}

bool CCiMulticastSocket::ReceiveInt32FromRecvBuffer(int &iReceivedIntMessage)
{
	int iTempInt32Message;

	if ( m_iRecvBufferPosition + sizeof(int) > m_iNBytesReceived ) {
		return false;
	}

	memcpy(&iTempInt32Message, m_recvBuffer+m_iRecvBufferPosition, sizeof(int));
	m_iRecvBufferPosition += 4;

	iReceivedIntMessage = ntohl( iTempInt32Message );

	return true;
}

bool CCiMulticastSocket::ReceiveNFromRecvBuffer(char *recvBuffer, int nBytesToReceive, int *nReceivedBytes)
{
	if ( m_iRecvBufferPosition + nBytesToReceive > m_iNBytesReceived ) {
		return false;
	}

	memcpy(recvBuffer, m_recvBuffer+m_iRecvBufferPosition, nBytesToReceive);
	m_iRecvBufferPosition += nBytesToReceive;

	if ( nReceivedBytes != NULL ) {
		*nReceivedBytes = nBytesToReceive;
	}

	return true;
}

bool CCiMulticastSocket::ClearRecvBuffer()
{
	m_iRecvBufferPosition = 0;
	m_iNBytesReceived = 0;

	return true;
}


bool CCiMulticastSocket::ClearSendBuffer()
{
	m_iSendBufferPosition = 0;
	return true;
}

bool CCiMulticastSocket::SendInt32ToSendBuffer(unsigned int uiValue)
{
	unsigned int uiValueToSend = htonl(uiValue);
	return SendNToSendBuffer(&uiValueToSend, sizeof(int));
}

bool CCiMulticastSocket::SendNToSendBuffer(const void* buf, int size)
{
	if ( m_iSendBufferPosition + size > sizeof(m_sendBuffer) ) {
		return false;
	}

	memcpy(m_sendBuffer+m_iSendBufferPosition, buf, size);
	m_iSendBufferPosition += size;

	return true;
}

bool CCiMulticastSocket::FlushSendBuffer()
{
	return SendN(m_sendBuffer, m_iSendBufferPosition);
}
